#pragma once
#include "mth_hull.h"

#define NUM_TEST_RECTS	(16)

class Engine
{
public:
	Engine(void);
	~Engine(void);

public:
	void Draw();

private:
	void DrawWorkspace();
	void DrawBench();
	void DrawSplit( int splitType, int location );
	void CreateTestRects( rect_list_type *list, _abb *rects, int num_rects );

private:
	mth_knapsack_type	m_knapsack;
	rect_list_type		m_test_rect_list;
	_abb				m_test_rects[NUM_TEST_RECTS];
};
