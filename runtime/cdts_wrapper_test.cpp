#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "cdts_wrapper.h"
#include <vector>
#include <cstring>

using namespace metaffi::runtime;


metaffi_int32**** init_4d_ragged_c_array()
{
	int elem = 1;
	// Initialize a 4D ragged array with size 2 for each dimension
	metaffi_int32**** arr = new metaffi_int32***[1];
	for (int i = 0; i < 1; ++i)
	{
		arr[i] = new metaffi_int32**[2];
		for (int j = 0; j < 2; ++j)
		{
			arr[i][j] = new metaffi_int32*[3];
			for (int k = 0; k < 3; ++k)
			{
				arr[i][j][k] = new metaffi_int32[4];
				for (int l = 0; l < 4; ++l)
				{
					// Assign some values to the 1D arrays
					arr[i][j][k][l] = elem;
					elem++;
				}
			}
		}
	}
	return arr;
}


TEST_CASE( "1D array", "[sdk]" )
{
	// Initialize a 4D ragged C-array
	metaffi_uint8 c_array[] = {1, 2, 3, 4, 5, 6, 7, 8};
	
	// Construct cdt_metaffi_int32_array using construct_multidim_array_bfs
	cdt_metaffi_uint8_array arr = {nullptr};
	construct_multidim_array<cdt_metaffi_uint8_array, metaffi_uint8>(arr, 1, c_array,
	 [](metaffi_size* index, metaffi_size index_length, void* other_array)->metaffi_size
	 {
		 FAIL("Shouldn't reach here");
		 return 0;
	 },
	 [](metaffi_size* index, metaffi_size index_length, metaffi_size& out_1d_array_length, void* other_array)->metaffi_uint8*
	 {
	     // Cast other_array to the correct type
		 
	     // Return the 1D array at the given index and its length
	     // Since we know that each dimension of the array has a size of 2, we return 2 as the length
	     out_1d_array_length = 8;
	     return static_cast<metaffi_uint8*>(other_array);
	 });
	
	
	traverse_multidim_array<cdt_metaffi_uint8_array, metaffi_uint8>(arr, (void*)c_array,
	[](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimension, metaffi_size array_length, void* other_array)
	{
	    FAIL("Shouldn't reach here");
	},
	[](metaffi_size* index, metaffi_size index_size, metaffi_uint8* arr, metaffi_size length, void* other_array)
	{
	    // Compare the values of the 1D array at the given index with arr
	    for (metaffi_size i = 0; i < length; ++i)
	    {
	        REQUIRE(arr[i] == ((metaffi_uint8*)other_array)[i]);
	    }
	});
	
}
//--------------------------------------------------------------------



TEST_CASE( "ragged array", "[sdk]" )
{
	// Initialize a 4D ragged C-array
	metaffi_int32**** c_array = init_4d_ragged_c_array();
	
	// Construct cdt_metaffi_int32_array using construct_multidim_array_bfs
	cdt_metaffi_int32_array arr = {nullptr};
	construct_multidim_array<cdt_metaffi_int32_array, metaffi_int32>(arr, 4, c_array,
	 [](metaffi_size* index, metaffi_size index_length, void* other_array)->metaffi_size
	 {
		 // Cast other_array to the correct type
		 metaffi_int32**** cur = static_cast<metaffi_int32****>(other_array);
		 
		 // Traverse the C-array according to the index
		 for (metaffi_size i = 0; i < index_length; i++)
		 {
			 cur = reinterpret_cast<metaffi_int32****>(cur[index[i]]);
		 }
		 
		 // the index_length is the length of the array at the given index
		 // (for array returned by init_4d_ragged_c_array)
		 return index_length+1;
	 },
	 [](metaffi_size* index, metaffi_size index_length, metaffi_size& out_1d_array_length, void* other_array)->metaffi_int32*
     {
	     // Cast other_array to the correct type
	     metaffi_int32**** cur = static_cast<metaffi_int32****>(other_array);
	     
	     // Traverse the C-array according to the index
		 for (metaffi_size i = 0; i < index_length; ++i)
	     {
			 cur = reinterpret_cast<metaffi_int32****>(cur[index[i]]);
	     }
		 
	     // Return the 1D array at the given index and its length
	     // Since we know that each dimension of the array has a size of 2, we return 2 as the length
	     out_1d_array_length = 4;
	     return reinterpret_cast<metaffi_int32*>(cur);
	 });
	
	
	 traverse_multidim_array<cdt_metaffi_int32_array, metaffi_int32>(arr, (void*)c_array,
	 [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimension, metaffi_size array_length, void* other_array)
	 {
		 // Cast other_array to the correct type
		 metaffi_int32**** cur = static_cast<metaffi_int32****>(other_array);
		 
		 // Traverse the C-array according to the index
		 for (metaffi_size i = 0; i < index_size; ++i)
		 {
			 cur = reinterpret_cast<metaffi_int32****>(cur[index[i]]);
		 }
		 
		 // Compare the length of the array at the given index with array_length
		 // Since we know that each dimension of the array has a size of 2, we expect array_length to be 2
		 switch(index_size)
		 {
			 case 0:
			 {
				 REQUIRE(array_length == 1);
				 break;
			 }
			 case 1:
			 {
				 REQUIRE(array_length == 2);
				 break;
			 }
			 case 2:
			 {
				 REQUIRE(array_length == 3);
				 break;
			 }
			 case 3:
			 {
				 REQUIRE(array_length == 4);
				 break;
			 }
			 default:
			 {
				 FAIL("Shouldn't reach here");
			 }
		 }
		 
	 },
	 [](metaffi_size* index, metaffi_size index_size, metaffi_int32* arr, metaffi_size length, void* other_array)
	 {
		 // Cast other_array to the correct type
		 metaffi_int32**** cur = static_cast<metaffi_int32****>(other_array);
		 
		 // Traverse the C-array according to the index
		 for (metaffi_size i = 0; i < index_size; ++i)
		 {
			 cur = reinterpret_cast<metaffi_int32****>(cur[index[i]]);
		 }
		 
		 // Compare the values of the 1D array at the given index with arr
		 for (metaffi_size i = 0; i < length; ++i)
		 {
			 REQUIRE(arr[i] == ((metaffi_int32*)cur)[i]);
		 }
	 });
	
}
//--------------------------------------------------------------------
