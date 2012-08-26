#ifndef MTH_HULL_H
#define MTH_HULL_H

////////////////////////
// Constants
////////////////////////

enum
{
	MAX_SPLIT_WIDTH	= 512,
	MIN_RECT_DIM = 16,
	MAX_SLABS		= MAX_SPLIT_WIDTH,
	CELL_STRIDE	    = 512/8,
	MAX_RECTS		= 16,
	WIDTH_SORT		= 0,
	HEIGHT_SORT		= 1,
	STATE_FOUND_BEST	= -999,
	STATE_FIND_NEXT		= -888,
	SPLIT_COLUMN	= 0,
	SPLIT_ROW
};


////////////////////////
// Types
////////////////////////

typedef uint8 cell_block_type; 

struct rect_list_item_type
	{
	_abb			   *item;
	rect_list_item_type *next;
	rect_list_item_type *prev;
	};

typedef struct
	{
	rect_list_item_type arr[MAX_RECTS];
	rect_list_item_type *head;
	rect_list_item_type *tail;
	} rect_list_type;

typedef struct
	{
	_abb				current_bounds;
	uint16				widest_rect_width;
	cell_block_type		cells[CELL_STRIDE][MAX_SLABS];
	uint16				num_columns;
	uint16				num_rows;
	uint16				column_extents[MAX_SLABS];
	uint16				row_extents[MAX_SLABS];
	uint32				calc_state;
	rect_list_type		list;
	uint16				best_score;
	_abb				best_bounds;
	} mth_knapsack_type;

////////////////////////
// Functions
////////////////////////

boolean func( _abb *rects, uint8 num_rects, mth_knapsack_type *ks );
void sort_rects_desc( rect_list_type *list, int test_type );
void add_rect_to_list_head( rect_list_type *list, _abb *item );

extern __inline boolean cell_is_occupied( uint16 x, uint16 y, mth_knapsack_type* ks );

#endif /* #ifndef MTH_HULL_H */