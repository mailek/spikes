#include "StdAfx.h"
#include "Engine.h"
#include <math.h>
#include "mth_hull.h"

enum {
	SPLIT_HORZ,
	SPLIT_VERT
} ESplitType;

static const _color RECT_COLOR = C_CYAN;
static const _color RECT_BORDER = C_PURPLE;
static const int CHECKER_BORDER_WIDTH = 20;
static const int BOUNDS_BORDER_WIDTH = 3;
static const int TEXTURE_EXTENT = 512;
static const int BENCH_BORDER_WIDTH = 20;
static const _color BENCH_COLOR = C_CANARY;

Engine::Engine(void)
{
	memset(&m_knapsack, 0, sizeof(m_knapsack));
	m_knapsack.current_bounds.extent[0] = TEXTURE_EXTENT;
	m_knapsack.current_bounds.extent[1] = TEXTURE_EXTENT;

	memset( &m_test_rects, 0, sizeof(m_test_rects) );
	CreateTestRects( &m_test_rect_list, m_test_rects, NUM_TEST_RECTS );

	// TODO - remove testing
	func( m_test_rects, NUM_TEST_RECTS, &m_knapsack );

}

Engine::~Engine(void)
{
}

void Engine::Draw()
{
	glLoadIdentity();

	/* Draw work area */
	DrawWorkspace();

	/* Draw Part Bench */
	DrawBench();
}

void Engine::DrawWorkspace()
{
	glPushMatrix();
	glTranslatef(50, 50, 0);
	_abb bnd = m_knapsack.current_bounds;

	/* Draw the texture checker board */
	_abb r;
	r.pos[0] = 0;
	r.pos[1] = 0;
	r.extent[0] = TEXTURE_EXTENT;
	r.extent[1] = TEXTURE_EXTENT;
	DrawCheckerBoard( r );

	/* Draw the texture matte */
	static const int borderSize = CHECKER_BORDER_WIDTH;
	_color checkerBorder = C_CANARY;
	r.pos[0] = -borderSize;
	r.pos[1] = -borderSize;
	r.extent[0] += 2*borderSize;
	r.extent[1] += 2*borderSize;
	DrawRectangleBorder( r, (float)borderSize, checkerBorder );

	/* Draw unused texture space */
	_color4 unusedColor = UNUSED_TEXTURE_COLOR;
	if( bnd.pos[0] + bnd.extent[0] < TEXTURE_EXTENT )
	{
		/* right slab */
		r.pos[0] = bnd.pos[0] + bnd.extent[0];
		r.pos[1] = bnd.pos[1];
		r.extent[0] = TEXTURE_EXTENT - bnd.extent[0];
		r.extent[1] = TEXTURE_EXTENT;
		DrawRectFilledBlend( r, unusedColor );
	}
	if( bnd.pos[1] + bnd.extent[1] < TEXTURE_EXTENT )
	{
		/* corner slab */
		r.pos[0] = bnd.pos[0];
		r.pos[1] = bnd.pos[1] + bnd.extent[1];
		r.extent[0] = bnd.extent[0] < TEXTURE_EXTENT ? bnd.extent[0] : TEXTURE_EXTENT;
		r.extent[1] = TEXTURE_EXTENT - bnd.extent[1];
		DrawRectFilledBlend( r, unusedColor );
	}

	/* Draw overrage texture space */
	_color4 overrageColor = OVERRAGE_TEXTURE_COLOR;
	if( bnd.pos[0] + bnd.extent[0] > TEXTURE_EXTENT )
	{
		/* right slab */
		r.pos[0] = TEXTURE_EXTENT;
		r.pos[1] = 0;
		r.extent[0] = bnd.extent[0] - TEXTURE_EXTENT;
		r.extent[1] = bnd.extent[1];
		
		int shiftMod = r.extent[0] % CHECKER_BLOCK_SIZE > 0 ? 1 : 0;
		r.extent[0] = (r.extent[0] / CHECKER_BLOCK_SIZE);
		r.extent[0] *= CHECKER_BLOCK_SIZE;
		r.extent[0] += shiftMod * CHECKER_BLOCK_SIZE;
		shiftMod = r.extent[1] % CHECKER_BLOCK_SIZE > 0 ? 1 : 0;
		r.extent[1] = (r.extent[1] / CHECKER_BLOCK_SIZE);
		r.extent[1] *= CHECKER_BLOCK_SIZE;
		r.extent[1] += shiftMod * CHECKER_BLOCK_SIZE;
		DrawCheckerBoard( r );
		
		r.extent[0] = bnd.extent[0] - TEXTURE_EXTENT;
		r.extent[1] = bnd.extent[1];
		DrawRectFilledBlend( r, overrageColor );
	}
	if( bnd.pos[1] + bnd.extent[1] > TEXTURE_EXTENT )
	{
		/* right slab */
		r.pos[0] = 0;
		r.pos[1] = TEXTURE_EXTENT;
		r.extent[1] = bnd.extent[1] - TEXTURE_EXTENT;

		int shiftMod = r.extent[1] % CHECKER_BLOCK_SIZE > 0 ? 1 : 0;
		r.extent[1] = (r.extent[1] / CHECKER_BLOCK_SIZE);
		r.extent[1] *= CHECKER_BLOCK_SIZE;
		r.extent[1] += shiftMod * CHECKER_BLOCK_SIZE;
		r.extent[0] = TEXTURE_EXTENT;
		DrawCheckerBoard( r );

		r.extent[0] = TEXTURE_EXTENT;
		r.extent[1] = bnd.extent[1] - TEXTURE_EXTENT;
		DrawRectFilledBlend( r, overrageColor );
	}

	/* Draw unused cells inside bounds */
	_color4 unusedCell = UNFILLED_CELL_COLOR;
	int x, y;
	y = 0;
	for( uint32 j = 0; j < m_knapsack.num_rows; j++ )
	{
		x = 0;

		for( uint32 i = 0; i < m_knapsack.num_columns; i++ )
		{
			if( cell_is_occupied( i, j, &m_knapsack ) )
			{
				r.pos[0] = x;
				r.extent[0] = m_knapsack.column_extents[i];
				r.pos[1] = y;
				r.extent[1] = m_knapsack.row_extents[j];
				//DrawRectFilledBlend( r, unusedCell );
			}

			x += m_knapsack.column_extents[i];
		}

		y += m_knapsack.row_extents[j];
	}

	/* Draw Horizontal and Vertical Splits */
	int pos = 0;
	for( uint32 i = 0; i < m_knapsack.num_columns; i++ )
	{
		pos += m_knapsack.column_extents[i];
		DrawSplit( SPLIT_VERT, pos);
	}

	pos = 0;
	for( uint32 i = 0; i < m_knapsack.num_rows; i++ )
	{
		pos += m_knapsack.row_extents[i];
		DrawSplit( SPLIT_HORZ, pos );
	}

	/* Draw the current bounds */
	_color boundsBorder = C_RED_APPLE;
	static const int boundsBorderSize = BOUNDS_BORDER_WIDTH;
	r = bnd;
	r.pos[0] -= boundsBorderSize;
	r.pos[1] -= boundsBorderSize;
	r.extent[0] += 2*boundsBorderSize;
	r.extent[1] += 2*boundsBorderSize;
	DrawRectangleBorder( r, (float)boundsBorderSize, boundsBorder );

	/* Draw the rectangles */
	_color inputRectBorder = C_BLUE;
	_color inputRectColor = C_CYAN;
	_color4 inputRectColor4;
	inputRectColor4.r = inputRectColor.r;
	inputRectColor4.g = inputRectColor.g;
	inputRectColor4.b = inputRectColor.b;
	inputRectColor4.a = 0.5f;
	
	for( int i = 0; i < NUM_TEST_RECTS; i++ )
	{
		//DrawRectangleFilled( m_test_rects[i], inputRectColor );
		DrawRectFilledBlend( m_test_rects[i], inputRectColor4 );
		DrawRectangleBorder( m_test_rects[i], 1, inputRectBorder );
	}

	glPopMatrix();
}

void Engine::DrawBench()
{
	rect_list_item_type* it;

	glPushMatrix();
	glTranslatef(50, 650, 0);
	
	/* Draw the matte */
	static const int matte_spacing = 5;
	int matte_width = 0;
	int matte_height = 0;
	it = m_test_rect_list.head;
	if( it )
	{
		matte_height = it->item->extent[1];
	}
	while( it )
	{
		matte_width += matte_spacing + it->item->extent[0];
		it = it->next;
	}

	_abb r;
	r.pos[0] = -BENCH_BORDER_WIDTH;
	r.pos[1] = -BENCH_BORDER_WIDTH;
	r.extent[0] = 2*BENCH_BORDER_WIDTH + matte_width;
	r.extent[1] = 2*BENCH_BORDER_WIDTH + matte_height;
	_color c = BENCH_COLOR;

	DrawRectangleFilled( r, c );

	/* Draw the rectangles */
	static const _color rectBorder = C_LIGHT_GRAY;
	static _color rectFill = C_ORANGE;

	it = m_test_rect_list.head;
	while( it )
	{
		r.pos[0] = 0;
		r.pos[1] = 0;
		r.extent[0] = it->item->extent[0];
		r.extent[1] = it->item->extent[1];

		DrawRectangleFilled( r, rectFill );
		DrawRectangleBorder( r, 1, rectBorder );
		glTranslatef( (float)it->item->extent[0] + matte_spacing, 0, 0 );
		it = it->next;
	}

	glPopMatrix();
}

void Engine::DrawSplit( int splitType, int location )
{
	static const int splitOverlap = 5;
	static const int borderSize = CHECKER_BORDER_WIDTH;
	_color splitColor = {0.8f, 0.1f, 0.8f};
	_abb r;
	_abb bnd = m_knapsack.current_bounds;

	switch(splitType)
	{
	case SPLIT_VERT:
		r.pos[0] = location - 1;
		r.pos[1] = -borderSize - splitOverlap;
		r.extent[0] = 2;
		r.extent[1] = borderSize + splitOverlap - r.pos[1] + (bnd.extent[1] > TEXTURE_EXTENT ? bnd.extent[1] : TEXTURE_EXTENT);
		break;
	case SPLIT_HORZ:
		r.pos[0] = -borderSize - splitOverlap;
		r.pos[1] = location - 1;
		r.extent[0] = borderSize + splitOverlap - r.pos[0] + (bnd.extent[0] > TEXTURE_EXTENT ? bnd.extent[0] : TEXTURE_EXTENT);
		r.extent[1] = 2;
		break;
	}

	DrawRectFilledBurn( r, splitColor );
}

void Engine::CreateTestRects( rect_list_type *list, _abb *rects, int num_rects )
{
#define MAX_RECTANGLE_WIDTH  200
#define MAX_RECTANGLE_HEIGHT 200
#define MIN_RECTANGLE_WIDTH 5
#define MIN_RECTANGLE_HEIGHT 5

	int i;
	memset( list, 0, sizeof(rect_list_type) );

	/* generate a bunch of random rects */
	for( i = 0; i < num_rects; i++ )
	{
		float random;
		random = (float)rand() / (float)RAND_MAX;
		rects[i].extent[0] = MIN_RECTANGLE_WIDTH + (int)(random * ( MAX_RECTANGLE_WIDTH - MIN_RECTANGLE_WIDTH ));

		random = (float)rand() / (float)RAND_MAX;
		rects[i].extent[1] = MIN_RECTANGLE_HEIGHT + (int)(random * ( MAX_RECTANGLE_HEIGHT - MIN_RECTANGLE_HEIGHT ));

		add_rect_to_list_head( list, &rects[i] );
	}

	sort_rects_desc( list, HEIGHT_SORT );
}