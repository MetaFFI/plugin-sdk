#include "cdts_wrapper.h"
#include <vector>
#include <cstring>

using namespace openffi::runtime;

int main()
{
	try
	{
		
		
		cdts_wrapper cdts(14);
		
		std::vector<openffi_types> vec_types =
		{
			openffi_float64_type,
			openffi_float32_type,
			openffi_int8_type,
			openffi_int16_type,
			openffi_int32_type,
			openffi_int64_type,
			openffi_uint8_type,
			openffi_uint16_type,
			openffi_uint32_type,
			openffi_uint64_type,
			openffi_bool_type,
			openffi_string_type,
			openffi_string_array_type,
			openffi_uint8_array_type
		};
		
		openffi_float32 p1 = 2.71f;
		openffi_float64 p2 = 3.141592;
		openffi_int8 p3 = -10;
		openffi_int16 p4 = -20;
		openffi_int32 p5 = -30;
		openffi_int64 p6 = -40;
		openffi_uint8 p7 = 50;
		openffi_uint16 p8 = 60;
		openffi_uint32 p9 = 70;
		openffi_uint64 p10 = 80;
		openffi_bool p11 = 1;
		std::string p12("This is an input");
		size_t p12_len = p12.size();
		std::vector<openffi_string> p13 = {(char*) "element one", (char*) "element two"};
		std::vector<openffi_size> p13_sizes = {strlen("element one"), strlen("element two")};
		std::vector<openffi_size> p13_dimensions_lengths = {p13.size()};
		
		std::vector<openffi_uint8> p14 = {2, 4, 6, 8, 10};
		std::vector<openffi_size> p14_dimensions_lengths = {p14.size()};
		
		cdts_build_callbacks cbs
		(
			[&](void* values_to_set, int index, openffi_float32& val) { val = p1; },
			[&](void* values_to_set, int index, openffi_float32*& val, openffi_bool& free_required)
			{
				val = &p1;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_float32*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_float64& val) { val = p2; },
			[&](void* values_to_set, int index, openffi_float64*& val, openffi_bool& free_required)
			{
				val = &p2;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_float64*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_int8& val) { val = p3; },
			[&](void* values_to_set, int index, openffi_int8*& val, openffi_bool& free_required)
			{
				val = &p3;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_int8*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_int16& val) { val = p4; },
			[&](void* values_to_set, int index, openffi_int16*& val, openffi_bool& free_required)
			{
				val = &p4;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_int16*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_int32& val) { val = p5; },
			[&](void* values_to_set, int index, openffi_int32*& val, openffi_bool& free_required)
			{
				val = &p5;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_int32*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_int64& val) { val = p6; },
			[&](void* values_to_set, int index, openffi_int64*& val, openffi_bool& free_required)
			{
				val = &p6;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_int64*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_uint8& val) { val = p7; },
			[&](void* values_to_set, int index, openffi_uint8*& val, openffi_bool& free_required)
			{
				val = &p7;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_uint8*& array, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required)
			{
				array = &p14[0];
				dimensions_lengths = &p14_dimensions_lengths[0];
				dimensions = 1;
				free_required = false;
			},
			
			[&](void* values_to_set, int index, openffi_uint16& val) { val = p8; },
			[&](void* values_to_set, int index, openffi_uint16*& val, openffi_bool& free_required)
			{
				val = &p8;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_uint16*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_uint32& val) { val = p9; },
			[&](void* values_to_set, int index, openffi_uint32*& val, openffi_bool& free_required)
			{
				val = &p9;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_uint32*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_uint64& val) { val = p10; },
			[&](void* values_to_set, int index, openffi_uint64*& val, openffi_bool& free_required)
			{
				val = &p10;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_uint64*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_bool& val) { val = p11; },
			[&](void* values_to_set, int index, openffi_bool*& val, openffi_bool& free_required)
			{
				val = &p11;
				free_required = 0;
			},
			[&](void* values_to_set, int index, openffi_bool*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},
			
			[&](void* values_to_set, int index, openffi_string& val, openffi_size& s)
			{
				val = (char*) p12.c_str();
				s = p12_len;
			},
			[&](void* values_to_set, int index, openffi_string*& val, openffi_size*& s, openffi_bool& free_required) {},
			[&](void* values_to_set, int index, openffi_string*& array, openffi_size*& strings_lengths, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required)
			{
				array = &p13[0];
				strings_lengths = &p13_sizes[0];
				dimensions_lengths = &p13_dimensions_lengths[0];
				dimensions = 1;
				free_required = false;
			},
			
			[&](void* values_to_set, int index, openffi_string8& val, openffi_size& s) {},
			[&](void* values_to_set, int index, openffi_string8*& val, openffi_size*& s, openffi_bool& free_required) {},
			[&](void* values_to_set, int index, openffi_string8*&, openffi_size*&, openffi_size*&, openffi_size&, openffi_bool&) {},
			
			[&](void* values_to_set, int index, openffi_string16& val, openffi_size& s) {},
			[&](void* values_to_set, int index, openffi_string16*& val, openffi_size*& s, openffi_bool& free_required) {},
			[&](void* values_to_set, int index, openffi_string16*&, openffi_size*&, openffi_size*&, openffi_size&, openffi_bool&) {},
			
			[&](void* values_to_set, int index, openffi_string32& val, openffi_size& s) {},
			[&](void* values_to_set, int index, openffi_string32*& val, openffi_size*& s, openffi_bool& free_required) {},
			[&](void* values_to_set, int index, openffi_string32*&, openffi_size*&, openffi_size*&, openffi_size&, openffi_bool&) {}
		);
		
		cdts.build(&vec_types[0], vec_types.size(), nullptr, cbs);
		
		//--------------------------------------------------------------------


#define asset_and_throw(code)\
			if(!(code)){ throw std::runtime_error(#code" Failed"); }
		
		cdts_parse_callbacks cps
		(
			[&](void* values_to_set, int index, const openffi_float32& val) { asset_and_throw(val == p1); },
			[&](void* values_to_set, int index, const openffi_float32* val){ asset_and_throw(*val != p1); },
			[&](void* values_to_set, int index, const openffi_float32* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_float64& val) { asset_and_throw(val == p2); },
			[&](void* values_to_set, int index, const openffi_float64* val){ asset_and_throw(*val == p1); },
			[&](void* values_to_set, int index, const openffi_float64* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_int8& val) { asset_and_throw(val == p3); },
			[&](void* values_to_set, int index, const openffi_int8* val){ asset_and_throw(*val == p3); },
			[&](void* values_to_set, int index, const openffi_int8* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_int16& val) { asset_and_throw(val == p4); },
			[&](void* values_to_set, int index, const openffi_int16* val){ asset_and_throw(val == &p4); },
			[&](void* values_to_set, int index, const openffi_int16* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_int32& val) { asset_and_throw(val == p5); },
			[&](void* values_to_set, int index, const openffi_int32* val){ asset_and_throw(*val == p5); },
			[&](void* values_to_set, int index, const openffi_int32* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_int64& val) { asset_and_throw(val == p6); },
			[&](void* values_to_set, int index, const openffi_int64* val){ asset_and_throw(*val == p6); },
			[&](void* values_to_set, int index, const openffi_int64* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_uint8& val) { asset_and_throw(val == p7); },
			[&](void* values_to_set, int index, const openffi_uint8* val){ asset_and_throw(*val == p7); },
			[&](void* values_to_set, int index, const openffi_uint8* array, const openffi_size* dimensions_lengths, const openffi_size& dimensions)
			{
				asset_and_throw(array[0] == p14[0]);
				asset_and_throw(array[1] == p14[1]);
				asset_and_throw(array[2] == p14[2]);
				asset_and_throw(array[3] == p14[3]);
				asset_and_throw(array[4] == p14[4]);
				
				asset_and_throw(dimensions_lengths[0] == p14_dimensions_lengths[0]);
				
				asset_and_throw(dimensions == 1);
			},
			
			[&](void* values_to_set, int index, const openffi_uint16& val) { asset_and_throw(val == p8); },
			[&](void* values_to_set, int index, const openffi_uint16* val){ asset_and_throw(*val == p8); },
			[&](void* values_to_set, int index, const openffi_uint16* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_uint32& val) { asset_and_throw(val == p9); },
			[&](void* values_to_set, int index, const openffi_uint32* val){ asset_and_throw(*val == p9); },
			[&](void* values_to_set, int index, const openffi_uint32* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_uint64& val) { asset_and_throw(val == p10); },
			[&](void* values_to_set, int index, const openffi_uint64* val) { asset_and_throw(*val == p10); },
			[&](void* values_to_set, int index, const openffi_uint64* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_bool& val) { asset_and_throw(val == p11); },
			[&](void* values_to_set, int index, const openffi_bool* val){ asset_and_throw(*val == p11); },
			[&](void* values_to_set, int index, const openffi_bool* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},
			
			[&](void* values_to_set, int index, const openffi_string& val, const openffi_size& s)
			{
				asset_and_throw(p12 == val);
				asset_and_throw(s == p12.length());
			},
			[&](void* values_to_set, int index, const openffi_string* val, const openffi_size* s) {},
			[&](void* values_to_set, int index, const openffi_string* array, const openffi_size* strings_lengths, const openffi_size* dimensions_lengths, const openffi_size& dimensions)
			{
				asset_and_throw(strcmp(p13[0], array[0]) == 0);
				asset_and_throw(strcmp(p13[1], array[1]) == 0);
				
				asset_and_throw(p13_sizes[0] == strings_lengths[0]);
				asset_and_throw(p13_sizes[1] == strings_lengths[1]);
				
				asset_and_throw(p13_dimensions_lengths[0] == dimensions_lengths[0]);
				asset_and_throw(p13_dimensions_lengths[1] == dimensions_lengths[1]);
				
				asset_and_throw(dimensions == 1);
				
			},
			
			[&](void* values_to_set, int index, const openffi_string8& val, const openffi_size& s) {},
			[&](void* values_to_set, int index, const openffi_string8* val, const openffi_size* s) {},
			[&](void* values_to_set, int index, const openffi_string8*, const openffi_size*, const openffi_size*, const openffi_size&) {},
			
			[&](void* values_to_set, int index, const openffi_string16& val, const openffi_size& s) {},
			[&](void* values_to_set, int index, const openffi_string16* val, const openffi_size* s) {},
			[&](void* values_to_set, int index, const openffi_string16*, const openffi_size*, const openffi_size*, const openffi_size&) {},
			
			[&](void* values_to_set, int index, const openffi_string32& val, const openffi_size& s) {},
			[&](void* values_to_set, int index, const openffi_string32* val, const openffi_size* s) {},
			[&](void* values_to_set, int index, const openffi_string32*, const openffi_size*, const openffi_size*, const openffi_size&) {}
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