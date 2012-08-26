// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

#include <gl\gl.h>		
#include <gl\glu.h>

typedef BOOL			boolean;
typedef int				sint32;
typedef unsigned int	uint32;
typedef short			sint16;
typedef unsigned short	uint16;
typedef char			sint8;
typedef unsigned char	uint8;
typedef unsigned long	uint64;

/////////////////////////////////////
//   LITERAL CONSTANTS
/////////////////////////////////////
#define C_BLACK					{0,0,0}
#define C_WHITE					{1,1,1}
#define C_BLUE					{0,0,1}
#define C_RED					{1,0,0}
#define C_GREEN					{0,1,0}
#define C_YELLOW				{1,1,0}
#define	C_PURPLE				{1,0,1}
#define C_CYAN					{0,1,1}
#define C_DARK_GRAY 			{0.2f, 0.2f, 0.2f}
#define C_LIGHT_GRAY			{0.34f, 0.41f, 0.44f}
#define C_CANARY				{0.95f, 1.0f, 0.22f}
#define C_RED_APPLE				{0.96f, 0.21f, 0.07f}
#define C_ORANGE				{0.99f, 0.38f, 0.01f}

#define UNUSED_TEXTURE_COLOR	{0.7f, 0.7f, 0.9f, 0.85f}
#define OVERRAGE_TEXTURE_COLOR	{0.7f, 0.3f, 0.2f, 0.90f}
#define UNFILLED_CELL_COLOR		{0.11f, 0.78f, 0.22f, 0.35f}
#define CHECKER_BLOCK_SIZE		(32)

/////////////////////////////////////
//   TYPES
/////////////////////////////////////
typedef struct
{
	uint16 pos[2];		// x, y
	uint16 extent[2];	// width, height
} _abb;

typedef struct
{
	float r;
	float g;
	float b;
} _color;

typedef struct
{
	float r;
	float g;
	float b;
	float a;
} _color4;

/////////////////////////////////////
//   INLINES
/////////////////////////////////////
__inline void DrawRectangleFilled(const _abb rect, _color rectColor)
{
	float top = rect.pos[1];
	float left = rect.pos[0];
	float bottom = top + rect.extent[1];
	float right = left + rect.extent[0];

	glColor3f( rectColor.r, rectColor.g, rectColor.b );
	glBegin( GL_QUADS );                  // Draw Quad:
		glVertex2f( left, top );   // Top Left
		glVertex2f( left, bottom );   // Bottom Left
		glVertex2f( right, bottom );   // Bottom Right
		glVertex2f( right, top );   // Top Right
	glEnd(); 
}

__inline void DrawRectFilledBurn( _abb rect, _color color )
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_COLOR, GL_ONE_MINUS_DST_COLOR );

	DrawRectangleFilled( rect, color );

	glPopAttrib();
}

__inline void DrawRectFilledBlend( _abb rect, _color4 color )
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	float top = rect.pos[1];
	float left = rect.pos[0];
	float bottom = top + rect.extent[1];
	float right = left + rect.extent[0];

	glColor4f( color.r, color.g, color.b, color.a );
	glBegin( GL_QUADS );                  // Draw Quad:
		glVertex2f( left, top );   // Top Left
		glVertex2f( left, bottom );   // Bottom Left
		glVertex2f( right, bottom );   // Bottom Right
		glVertex2f( right, top );   // Top Right
	glEnd(); 

	glPopAttrib();
}

/* border width grows inward with lineWidth */
__inline void DrawRectangleBorder(const _abb rect, float lineWidth, _color lineColor)
{
	float top = rect.pos[1];
	float left = rect.pos[0];
	float bottom = top + rect.extent[1];
	float right = left + rect.extent[0]; 

	glLineWidth(lineWidth);
	glColor3f( lineColor.r, lineColor.g, lineColor.b );
	glBegin( GL_QUADS );                  
		glVertex2f( left, top );
		glVertex2f( left, top+lineWidth );
		glVertex2f( right, top+lineWidth );
		glVertex2f( right, top );

		glVertex2f( right-lineWidth, top );
		glVertex2f( right, top );
		glVertex2f( right, bottom );
		glVertex2f( right-lineWidth, bottom );

		glVertex2f( left, bottom );
		glVertex2f( left, bottom-lineWidth );
		glVertex2f( right, bottom-lineWidth );
		glVertex2f( right, bottom );

		glVertex2f( left+lineWidth, top );
		glVertex2f( left, top );
		glVertex2f( left, bottom );
		glVertex2f( left+lineWidth, bottom );
	glEnd(); 
}

__inline void DrawCheckerBlock(uint16 x, uint16 y)
{
	static const int box_size = CHECKER_BLOCK_SIZE/2;
	_color black = C_DARK_GRAY;
	_color white = C_WHITE;

	_abb box;
	box.pos[0] = x;
	box.pos[1] = y;
	box.extent[0] = box_size;
	box.extent[1] = box_size;

	DrawRectangleFilled( box, white );

	box.pos[0] += box_size;
	box.pos[1] += box_size;
	DrawRectangleFilled( box, white );

	box.pos[0] = x;
	DrawRectangleFilled( box, black );

	box.pos[1] = y;
	box.pos[0] += box_size;
	DrawRectangleFilled( box, black );
}

__inline void DrawCheckerBoard( _abb bounds )
{
	assert( ((bounds.extent[0] % CHECKER_BLOCK_SIZE) == 0) && ((bounds.extent[1] % CHECKER_BLOCK_SIZE) == 0) );
	
	for(int j = bounds.pos[1]; j < bounds.pos[1]+bounds.extent[1]; j+=CHECKER_BLOCK_SIZE )
	{
		for(int i = bounds.pos[0]; i < bounds.pos[0]+bounds.extent[0]; i+=CHECKER_BLOCK_SIZE )
		{
			DrawCheckerBlock(i, j);
		}
	}
}
