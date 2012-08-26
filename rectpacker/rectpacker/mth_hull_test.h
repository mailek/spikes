#include "mth_hull.h"

#if defined( _DEBUG )
#define _v( e )			\
	assert( e )
#endif

void test_split_slab( void );

#if defined( INCLUDE_TESTS )
void test_split_slab
	(
	void
	)
{
mth_knapsack_type ks;

memset( &ks, 0, sizeof(ks) );
ks.current_bounds.extent[0] = 512;
ks.current_bounds.extent[1] = 512;



//static void split_slab( uint16 split_idx, uint16 dim_left, int split_type, mth_knapsack_type *ks )

}


void test_set_cell_occupied()
{
	mth_knapsack_type ks;
	
	/* test cell 0,0 call be set and cleared */
	memset( &ks, 0, sizeof(ks) );
	
	set_cell_occupied( 0, 0, TRUE, &ks );
	


	//__inline void set_cell_occupied( uint16 x, uint16 y, boolean occupied, mth_knapsack_type* ks )
}

#endif /* INCLUDE_TESTS */

