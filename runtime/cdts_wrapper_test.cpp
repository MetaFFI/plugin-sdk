#include "cdts_wrapper.h"
#include "cdts_alloc.h"
#include <vector>
#include <cstring>

using namespace metaffi::runtime;

int main()
{
	try
	{
		cdt* pcdts = (cdt*)alloca(sizeof(cdt)*15);
		cdts_wrapper cdts(pcdts, 15);
		
		std::vector<metaffi_types> vec_types =
		{
			metaffi_float64_type,
			metaffi_float32_type,
			metaffi_int8_type,
			metaffi_int16_type,
			metaffi_int32_type,
			metaffi_int64_type,
			metaffi_uint8_type,
			metaffi_uint16_type,
			metaffi_uint32_type,
			metaffi_uint64_type,
			metaffi_bool_type,
			metaffi_handle_type,
			metaffi_string8_type,
			metaffi_string8_array_type,
			metaffi_uint8_array_type
		};
		
		metaffi_float32 p1 = 2.71f;
		metaffi_float64 p2 = 3.141592;
		metaffi_int8 p3 = -10;
		metaffi_int16 p4 = -20;
		metaffi_int32 p5 = -30;
		metaffi_int64 p6 = -40;
		metaffi_uint8 p7 = 50;
		metaffi_uint16 p8 = 60;
		metaffi_uint32 p9 = 70;
		metaffi_uint64 p10 = 80;
		metaffi_bool p11 = 1;
		std::string p12("This is an input");
		size_t p12_len = p12.size();
		std::vector<metaffi_string8> p13 = {(char*) "element one", (char*) "element two"};
		std::vector<metaffi_size> p13_sizes = {strlen("element one"), strlen("element two")};
		std::vector<metaffi_size> p13_dimensions_lengths = {p13.size()};
		
		std::vector<metaffi_uint8> p14 = {2, 4, 6, 8, 10};
		std::vector<metaffi_size> p14_dimensions_lengths = {p14.size()};
		
		metaffi_handle p15 = (void*)0xABDEF;
		
		cdts_build_callbacks cbs
		(
			[&](void* values_to_set, int index, metaffi_float32& val) { val = p1; },
			[&](void* values_to_set, int index, metaffi_float32*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_float64& val) { val = p2; },
			[&](void* values_to_set, int index, metaffi_float64*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_int8& val) { val = p3; },
			[&](void* values_to_set, int index, metaffi_int8*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_int16& val) { val = p4; },
			[&](void* values_to_set, int index, metaffi_int16*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_int32& val) { val = p5; },
			[&](void* values_to_set, int index, metaffi_int32*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_int64& val) { val = p6; },
			[&](void* values_to_set, int index, metaffi_int64*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_uint8& val) { val = p7; },
			[&](void* values_to_set, int index, metaffi_uint8*& array, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required)
			{
				array = &p14[0];
				dimensions_lengths = &p14_dimensions_lengths[0];
				dimensions = 1;
				free_required = false;
			},
			
			[&](void* values_to_set, int index, metaffi_uint16& val) { val = p8; },
			[&](void* values_to_set, int index, metaffi_uint16*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_uint32& val) { val = p9; },
			[&](void* values_to_set, int index, metaffi_uint32*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_uint64& val) { val = p10; },
			[&](void* values_to_set, int index, metaffi_uint64*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_bool& val) { val = p11; },
			[&](void* values_to_set, int index, metaffi_bool*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_handle& val) { val = (void*)0xABDEF; },
			[&](void* values_to_set, int index, metaffi_handle*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, metaffi_string8& val, metaffi_size& s)
			{
				val = (char*) p12.c_str();
				s = p12_len;
			},
			[&](void* values_to_set, int index, metaffi_string8*& array, metaffi_size*& strings_lengths, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required)
			{
				array = &p13[0];
				strings_lengths = &p13_sizes[0];
				dimensions_lengths = &p13_dimensions_lengths[0];
				dimensions = 1;
				free_required = false;
			},
			
			[&](void* values_to_set, int index, metaffi_string16& val, metaffi_size& s) {},
			[&](void* values_to_set, int index, metaffi_string16*&, metaffi_size*&, metaffi_size*&, metaffi_size&, metaffi_bool&) {},
			
			[&](void* values_to_set, int index, metaffi_string32& val, metaffi_size& s) {},
			[&](void* values_to_set, int index, metaffi_string32*&, metaffi_size*&, metaffi_size*&, metaffi_size&, metaffi_bool&) {}
		);
		
		cdts.build(&vec_types[0], vec_types.size(), nullptr, cbs);
		
		//--------------------------------------------------------------------


#define asset_and_throw(code)\
			if(!(code)){ throw std::runtime_error(#code" Failed"); }
		
		cdts_parse_callbacks cps
		(
			[&](void* values_to_set, int index, const metaffi_float32& val) { asset_and_throw(val == p1); },
			[&](void* values_to_set, int index, const metaffi_float32* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_float64& val) { asset_and_throw(val == p2); },
			[&](void* values_to_set, int index, const metaffi_float64* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_int8& val) { asset_and_throw(val == p3); },
			[&](void* values_to_set, int index, const metaffi_int8* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_int16& val) { asset_and_throw(val == p4); },
			[&](void* values_to_set, int index, const metaffi_int16* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_int32& val) { asset_and_throw(val == p5); },
			[&](void* values_to_set, int index, const metaffi_int32* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_int64& val) { asset_and_throw(val == p6); },
			[&](void* values_to_set, int index, const metaffi_int64* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_uint8& val) { asset_and_throw(val == p7); },
			[&](void* values_to_set, int index, const metaffi_uint8* array, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions)
			{
				asset_and_throw(array[0] == p14[0]);
				asset_and_throw(array[1] == p14[1]);
				asset_and_throw(array[2] == p14[2]);
				asset_and_throw(array[3] == p14[3]);
				asset_and_throw(array[4] == p14[4]);
				
				asset_and_throw(dimensions_lengths[0] == p14_dimensions_lengths[0]);
				
				asset_and_throw(dimensions == 1);
			},
			
			[&](void* values_to_set, int index, const metaffi_uint16& val) { asset_and_throw(val == p8); },
			[&](void* values_to_set, int index, const metaffi_uint16* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_uint32& val) { asset_and_throw(val == p9); },
			[&](void* values_to_set, int index, const metaffi_uint32* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_uint64& val) { asset_and_throw(val == p10); },
			[&](void* values_to_set, int index, const metaffi_uint64* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_bool& val) { asset_and_throw(val == p11); },
			[&](void* values_to_set, int index, const metaffi_bool* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_handle& val) { asset_and_throw(val == p15); },
			[&](void* values_to_set, int index, const metaffi_handle* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const metaffi_string8& val, const metaffi_size& s)
			{
				asset_and_throw(p12 == val);
				asset_and_throw(s == p12.length());
			},
			[&](void* values_to_set, int index, const metaffi_string8* array, const metaffi_size* strings_lengths, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions)
			{
				asset_and_throw(strcmp(p13[0], array[0]) == 0);
				asset_and_throw(strcmp(p13[1], array[1]) == 0);
				
				asset_and_throw(p13_sizes[0] == strings_lengths[0]);
				asset_and_throw(p13_sizes[1] == strings_lengths[1]);
				
				asset_and_throw(p13_dimensions_lengths[0] == dimensions_lengths[0]);
				asset_and_throw(p13_dimensions_lengths[1] == dimensions_lengths[1]);
				
				asset_and_throw(dimensions == 1);
				
			},

			[&](void* values_to_set, int index, const metaffi_string16& val, const metaffi_size& s) {},
			[&](void* values_to_set, int index, const metaffi_string16*, const metaffi_size*, const metaffi_size*, const metaffi_size&) {},
			
			[&](void* values_to_set, int index, const metaffi_string32& val, const metaffi_size& s) {},
			[&](void* values_to_set, int index, const metaffi_string32*, const metaffi_size*, const metaffi_size*, const metaffi_size&) {}
		);
		
		cdts.parse(nullptr, cps);
		
		
		return 0;
		
	}
	catch(std::exception& err)
	{
		printf("FATAL: %s\n", err.what());
		return 1;
	}
}