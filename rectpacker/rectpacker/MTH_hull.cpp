#include "stdafx.h"

#include "MTH_hull.h"

/************************************
			LITERALS
************************************/
#define CHAR_BITS		(8)

/************************************
			TYPES
************************************/
struct best_cell
	{
	uint16				top_left_col;
	uint16				top_left_row;
	uint8				right_merges;
	uint8				below_merges;
	};

enum {
	BOUNDS_FAILED,
	BOUNDS_SUCCEEDED
	};

/************************************
			FUNCTIONS
************************************/
static void split_slab( uint16 split_idx, uint16 dim_left, int split_type, mth_knapsack_type *ks );
static boolean find_best_unoccupied_cell( _abb rect, best_cell *best, mth_knapsack_type *ks );
static uint16 score_bounds( _abb b );
static void trim_bounds( mth_knapsack_type *ks );
static uint8 place_rects_in_bounds(  _abb *rects, int num_rects, mth_knapsack_type *ks );
static void update_bounds_score( mth_knapsack_type *ks );

/************************************
			INLINES
************************************/

__inline void set_cell_occupied( uint16 x, uint16 y, boolean occupied, mth_knapsack_type* ks )
{
int						xBlock;

xBlock = x / ( sizeof(cell_block_type) * 8 );
if( occupied )
	ks->cells[xBlock][y] |= ( 1 << ( x % ( sizeof(cell_block_type) * 8 ) ) );
else
	ks->cells[xBlock][y] &= ~( 1 << ( x % ( sizeof(cell_block_type) * 8 ) ) );

}

__inline boolean cell_is_occupied( uint16 x, uint16 y, mth_knapsack_type* ks )
{
int						xBlock;

xBlock = x / ( sizeof(cell_block_type) * 8 );

return ( ( ks->cells[xBlock][y] & ( 1 << ( x % ( sizeof(cell_block_type) * 8 ) ) ) ) != 0 );
}

/************************************
			TESTS
************************************/
#define INCLUDE_TESTS
#include "mth_hull_test.h"
#undef INCLUDE_TESTS

/************************************
			DEFINTIONS
************************************/

void add_rect_to_list_head( rect_list_type *list, _abb *item )
{
int						i;
rect_list_item_type    *insert;
rect_list_item_type    *old_head;

/* find an empty arr item */
insert = NULL;
for( i = 0; i < MAX_RECTS; i++ )
	{
	if( list->arr[i].item == NULL )
		{
		insert = &list->arr[i];
		break;
		}
	}
assert( insert );

/* fill out the item (sets as used) */
insert->item = item;
insert->prev = NULL;
insert->next = NULL;

/* insert at head of list */
old_head = list->head;
if( old_head )
	{
	list->head = insert;
	insert->next = old_head;
	old_head->prev = insert;
	}
else
	{
	/* first item in list */
	list->head = insert;
	list->tail = insert;
	}

}	/* add_rect_to_list_head() */


/* sort rects by either width or height using a simple insertion sort */
void sort_rects_desc( rect_list_type *list, int test_type )
{
rect_list_item_type	   *walk;
rect_list_item_type    *insert;
rect_list_item_type    *target;
int						compare_idx;
rect_list_item_type    *old_head;

walk = list->head;
insert = target = NULL;
compare_idx = ( test_type == WIDTH_SORT ) ? 0 : 1;

while( walk )
	{
	while( walk->next && walk->next->item->extent[compare_idx] > walk->item->extent[compare_idx] )
		{
		/* item is out of order */
		insert = walk->next;

		/* unhook the insert item */
		if( insert->next )
			{
			insert->next->prev = insert->prev;
			}
		else
			{
			assert( list->tail == insert );
			list->tail = insert->prev;
			}

		if( insert->prev )
			{
			insert->prev->next = insert->next;
			}
		
		insert->prev = insert->next = NULL;

		/* find the insert target */
		target = walk->prev;
		while( target )
			{
			if( target->item->extent[compare_idx] > insert->item->extent[compare_idx] )
				{
				/* insert item after target */
				if( list->tail == target )
					{
					list->tail = insert;
					}

				insert->next = target->next;
				insert->prev = target;

				if( target->next )
					{
					target->next->prev = insert;
					}
				target->next = insert;

				break;
				}

			target = target->prev;
			}

		if( target == NULL && list->head )
			{
			/* Must have hit the head of list, so insert there */ 
			old_head = list->head;
			list->head = insert;
			insert->next = old_head;
			insert->prev = NULL;
			old_head->prev = insert;
			}

		}
	
	walk = walk->next;
	}

}	/* sort_rects_desc() */


/* calculate the top-left corner position of the requested cell */
__inline void calc_cell_position( uint16 col, uint16 row, mth_knapsack_type *ks, uint16 *x_out, uint16 *y_out )
{
*x_out = 0;
*y_out = 0;

/* walk the slabs and calculate position as sum of extents */
for( int i = 0; i <= col; i++ )
	{
	*x_out += ks->column_extents[i];
	}

for( int i = 0; i <= row; i++ )
	{
	*y_out += ks->row_extents[i];
	}

}	/* calc_cell_position() */


/* main algorithm entry, input rects must be at least 16x16p */
boolean func( _abb *rects, uint8 num_rects, mth_knapsack_type *ks )
{
uint64					area;
rect_list_item_type	   *it;
uint32					i;

assert( num_rects <= MAX_RECTS );

switch( ks->calc_state )
{
case STATE_FOUND_BEST:
	break;
case STATE_FIND_NEXT:
	/* clear out the cells and splits */
	memset( ks->cells, 0, sizeof(ks->cells) );
	memset( ks->column_extents, 0, sizeof(ks->column_extents) );
	memset( ks->row_extents, 0, sizeof(ks->row_extents) );

	/* place the rectangles tightly in the current bounds,
	to find the best bounds */
	ks->num_columns = 1;
	ks->num_rows = 1;
	ks->column_extents[0] = ks->current_bounds.extent[0];
	ks->row_extents[0] = ks->current_bounds.extent[1];

	while( 1 )
		{
		uint8 result = place_rects_in_bounds( rects, num_rects, ks );
		
		if( result == BOUNDS_SUCCEEDED )
			{
			/* trim the current bounds */
			trim_bounds( ks );

			/* score the bounds */
			update_bounds_score( ks );

			/* set the knapsack as ready for further processing */
			ks->calc_state = STATE_FIND_NEXT;
			break;
			}

			// TODO: 
			/* try a skinnier/longer bounds */
				/* find the top-most rectangle that touches the right bounds edge */
				/* increase the current bounds height by the height of this rectangle */

	
			/* if we reached the skinniest bounds possible ( equal to width of widest rect ) then fail */
			if( ks->current_bounds.extent[0] < ks->widest_rect_width )
			{
				ks->calc_state = STATE_FOUND_BEST;
				break;
			}
		}
	break;
default:
	{
	/* Init the knapsack */
	memset( ks, 0, sizeof(mth_knapsack_type) );

	/* early out checks for if total rect space will fit in processing space */
	area = 0;	
	for( i = 0; i < num_rects; i++ )
		{
		area += (int)rects[i].extent[0] * (int)rects[i].extent[1];
		if( rects[i].extent[0] > MAX_SPLIT_WIDTH || 
			rects[i].extent[0] < MIN_RECT_DIM || 
			rects[i].extent[1] > MAX_SPLIT_WIDTH ||
			rects[i].extent[1] < MIN_RECT_DIM )
			{
			return FALSE;
			}
		}

	if( area > MAX_SPLIT_WIDTH * MAX_SPLIT_WIDTH )
		{
		return FALSE;
		}

	/* create the abb list for processing */
	memset( &ks->list, 0, sizeof(ks->list) );
	for( i = 0; i < num_rects; i++ )
		{
		add_rect_to_list_head( &ks->list, &rects[i] );
		}

	/* start with the rectangles sort by tallest to shortest */
	sort_rects_desc( &ks->list, HEIGHT_SORT );

	/* find the widest rectangle for exit determination */
	it = ks->list.head;
	while( it )
		{
		ks->widest_rect_width = max( ks->widest_rect_width, it->item->extent[0] );	
		it = it->next;
		}

	/* set the initial height to the height of the tallest rectangle */
	it = ks->list.head;
	if( it )
		{
		ks->num_rows = 1;
		ks->row_extents[0] = (uint16)it->item->extent[1];
		}

	/* then add each rectangle to a new knapsack column */
	uint16 rect_x = 0;
	while( it )
		{
		/* update rectangle position */
		it->item->pos[0] = rect_x;
		it->item->pos[1] = 0;

		/* update column */
		ks->column_extents[ks->num_columns] = (int)it->item->extent[0];
		rect_x += ks->column_extents[ks->num_columns];
		ks->num_columns++;

		/* next rectangle */
		it = it->next;
		}

	/* update the bounds with the initial bounds 
	( height = tallest rect, width = sum of rect widths */
	for( i = 0; i < ks->num_columns; i++ )
		ks->current_bounds.extent[0] += ks->column_extents[i];

	for( i = 0; i < ks->num_rows; i++ )
		ks->current_bounds.extent[1] += ks->row_extents[i];

	/* score the initial bounds */
	update_bounds_score( ks );

	}
	break;
}

return TRUE;
}	/* func() */

/* splits a slab with the 'previous' slab size being dim_left, and the new slab of the remaining size */
static void split_slab( uint16 split_idx, uint16 dim_left, int split_type, mth_knapsack_type *ks )
{
uint16					dim_right;
sint32					i;
sint32					j;
boolean					col_overflow;
uint32					cell_mask;
cell_block_type			cell;
uint16					x_block;
uint8					x_offset;
uint32					num_active_cols;

const uint8 cell_width = sizeof(cell_block_type)*CHAR_BITS;

if( split_type == SPLIT_COLUMN )
	{
	assert( ks->num_columns > split_idx );
	assert( ks->column_extents[split_idx] > dim_left);

	/* split column extents */		
	dim_right = ks->column_extents[split_idx] - dim_left;
	ks->column_extents[split_idx] = dim_left;
	
	/* shift all the columns to the right, making room for the new column */
	for( i = ks->num_columns; i > split_idx+1; i-- )
		{
		ks->column_extents[i] = ks->column_extents[i-1];
		}

	/* set the new column's size */
	ks->column_extents[split_idx+1] = dim_right;

	/* shift each cell row */
	x_block = split_idx / cell_width;
	x_offset = split_idx % cell_width;
	col_overflow = FALSE;
	
	cell_mask = (~0) << x_offset;
	for( j = 0; j < ks->num_rows; j++ )
		{
		/* split cells by shifting each right outlying bit rightward */
		for( i = ks->num_columns-1; i > x_block; i-- )
			{
			col_overflow = ks->cells[i][j] & ( 1 << ( cell_width - 1 ) );
			if( col_overflow )
				{
				ks->cells[i+1][j] |= 1;
				col_overflow = FALSE;
				}

			ks->cells[i][j] <<= 1;
			}

		/* shift the final cell, preserving left outlying bits, and setting 
		the new column to its parent's occupied state */
		col_overflow = ks->cells[x_block][j] & ( 1 << ( cell_width - 1 ) );
		if( col_overflow )
			{
			ks->cells[x_block+1][j] |= 1;
			col_overflow = FALSE;
			}

		cell = ks->cells[x_block][j];
		cell = ( cell & cell_mask ) | ( ( cell & ~cell_mask ) << 1 );
		ks->cells[x_block][j] = cell;
		}

	/* finally increment the column count */
	ks->num_columns += 1;
	}
else
	{
	assert( split_type == SPLIT_ROW );
	assert( ks->num_rows > split_idx );
	assert( ks->row_extents[split_idx] > dim_left);

	/* split row extents */
	dim_right = ks->row_extents[split_idx] - dim_left;
	ks->row_extents[split_idx] = dim_left;
	
	for( i = ks->num_rows; i > split_idx+1; i-- )
		{
		ks->row_extents[i] = ks->row_extents[i-1];
		}

	ks->row_extents[split_idx+1] = dim_right;

	/* split cells */
	num_active_cols = ( ks->num_columns / cell_width) + 1;
	for( j = ks->num_rows-1; j > split_idx; j-- )
		{
		for( uint32 i = 0; i < /*num_active_rows*/num_active_cols; i++ )
			{
			ks->cells[i+1][j+1] = ks->cells[i][j];
			}
		}

	/* finally increment the row count */
	ks->num_rows += 1;
	}

}	/* split_slab() */


/* finds the left-most -> top-most cell that is unoccupied, and if necessary looks 
for cells to merge in order to fit rectangle */
static boolean find_best_unoccupied_cell( _abb rect, best_cell *best, mth_knapsack_type *ks )
{
uint32					i;
uint32					j;
uint16					x;
uint16					y;
uint8					col_merges;
uint8					row_merges;
sint32					slab_overage;
boolean					slab_merge_failed;

/* find the left-most column with an unoccupied cell */
for( i = 0; i < ks->num_columns; i++ )
	{
	for( j = 0; j < ks->num_rows; j++ )
		{
		if( !cell_is_occupied( i, j, ks ) )
			{
			/* check if rectangle will fit in cell */
			col_merges = row_merges = 0;

			if( ks->column_extents[i] >= rect.extent[0] && ks->row_extents[j] >= rect.extent[1] )
				{
				/* single cell will fit whole rectangle, easy out */
				best->below_merges = 0;
				best->right_merges = 0;
				best->top_left_col = i;
				best->top_left_row = j;

				return TRUE;
				}

			/* merge check across columns first, but only for first row */
			best->right_merges = 0;
			slab_overage = rect.extent[0] - ks->column_extents[i];
			slab_merge_failed = FALSE;
			x = 0;

			if( slab_overage > 0 )
				{
				/* scan to the right, until the whole rect width fits */
				for( ; slab_overage > 0; x++ )
					{
					if( cell_is_occupied( i+x, j, ks ) )
						{
						slab_merge_failed = TRUE;
						break;
						}

					slab_overage -= ks->column_extents[i+x];
					}	
				}

			/* if this cell merge failed, try the next one */
			if( slab_merge_failed )
				{
				continue;
				}

			best->right_merges += x+1;
		
			/* now merge the rows, only this time check all the merged-in column's cells as well */
			best->below_merges = 0;
			slab_merge_failed = FALSE;
			y = 0;
			if( ks->row_extents[j] < rect.extent[1] )
				{
				slab_overage = rect.extent[1] - ks->row_extents[j];
				for( ; slab_overage > 0 && !slab_merge_failed; y++ )
					{
					for( x = 0; x <= best->right_merges; x++ )
						{
						if( cell_is_occupied( i+x, j+y, ks ) )
							{
							slab_merge_failed = TRUE;
							break;
							}
						}

					slab_overage -= ks->row_extents[j+y];
					}
				}

			if( slab_merge_failed )
				{
				continue;
				}

			best->below_merges += y+1;

			/* we found an open space for the rectangle, and the merges have been logged, return success */
			return TRUE;
			}
		}
	}

/* we must have traversed the whole bounds and not 
	found an open place, return failure */
return FALSE;

}	/* find_best_unoccupied_cell() */


/* trims off unoccupied columns and row to obtain the tightest bounds */
static void trim_bounds( mth_knapsack_type *ks )
{
boolean					trimming;
uint16					i;
uint16					j;

assert( ks->num_columns > 0 && ks->num_rows > 0 );

/* trim the columns */
trimming = TRUE;

/* check each column for vacancy, starting from right-most */
for( i = ks->num_columns-1; i >= 1 && trimming; i-- )
	{
	for( j = 0; j < ks->num_rows && trimming; j++ )
		{
		if( cell_is_occupied( i, j, ks ) )
			{
			trimming = FALSE;
			}
		}

	/* trim the column if it is vacant */
	if( trimming )
		{
		ks->column_extents[ks->num_columns-1] = 0;
		ks->num_columns -= 1;
		}
	}

/* trim the rows */
trimming = TRUE;

/* check each row for vacancy, starting from bottom-most */
for( i = ks->num_rows-1; i >= 1 && trimming; i-- )
	{
	for( j = 0; j < ks->num_columns && trimming; j++ )
		{
		if( cell_is_occupied( i, j, ks ) )
			{
			trimming = FALSE;
			}
		}

	/* trim the row if it is vacant */
	if( trimming )
		{
		ks->row_extents[ks->num_rows-1] = 0;
		ks->num_rows -= 1;
		}
	}

}	/* trim_bounds() */


/* attempts to place all rectangles in the current bounds, preferring left-most -> top-most placement */
static uint8 place_rects_in_bounds( _abb *rects, int num_rects, mth_knapsack_type *ks )
{
best_cell best;
rect_list_item_type	   *it;
uint32					i;
uint32					j;
uint16					right_edge_col;
uint16					bottom_edge_row;

it = ks->list.head;
while( it )
	{
	/* find left-most -> top-most unoccupied cell */
	if( !find_best_unoccupied_cell( *it->item, &best, ks ) )
		{
		/* failed to place a rectangle in the current bounds - this bounds has failed */
		return BOUNDS_FAILED;
		}

	/* the the bottom-right rectangles as occupied and split slabs if needed */
	right_edge_col = best.top_left_col + best.right_merges;
	if( ks->column_extents[right_edge_col] > it->item->extent[0] )
		{
		/* split the column */
			split_slab( right_edge_col, it->item->extent[0], SPLIT_COLUMN, ks );
		}

	bottom_edge_row = best.top_left_row + best.below_merges;
	if( ks->row_extents[bottom_edge_row] > it->item->extent[1] )
		{
		/* split the row */
		split_slab( bottom_edge_row, it->item->extent[1], SPLIT_ROW, ks );
		}
	
	/* set all the rows as occupied */
	for( j = 0; j <= best.below_merges; j++ )
		{
		/* set all the cols as occupied */
		for( i = 0; i <= best.right_merges; i++ )
			{
			set_cell_occupied( best.top_left_col+i, best.top_left_row+j, TRUE, ks );
			}
		}

	/* update the rectangle's position */
	calc_cell_position( best.top_left_col, best.top_left_row, ks, &it->item->pos[0], &it->item->pos[1] );

	/* go to the next rectangle */
	it = it->next;
	}

return BOUNDS_SUCCEEDED;

}	/* place_rects_in_bounds() */


/* scores the current bounds, and if better score than previous 
	best, then save bounds and rectangle placement */
static void update_bounds_score( mth_knapsack_type *ks )
{
uint16					score;

score = score_bounds( ks->current_bounds );
if( score > ks->best_score )
	{
	ks->best_score = score;
	ks->best_bounds = ks->current_bounds;
	}

}	/* update_bounds_score() */


/* scores a successful bounds relative to the algorithm's configured rules */
static uint16 score_bounds( _abb b )
{
// TODO
return 0;

}	/* score_bounds() */
