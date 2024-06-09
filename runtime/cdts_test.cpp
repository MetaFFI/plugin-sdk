#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../utils/scope_guard.hpp"
#include "cdts_traverse_construct.h"
#include <doctest/doctest.h>
#include <utility>
#include <vector>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

using namespace metaffi::runtime;

TEST_SUITE("CDTS Tests")
{
	TEST_CASE("MetaFFI primitives")
	{
		metaffi_float32 f32 = 3.14f;
		metaffi_float64 f64 = 2.71828;
		metaffi_int8 i8 = 1;
		metaffi_uint8 ui8 = 2;
		metaffi_int16 i16 = 3;
		metaffi_uint16 ui16 = 4;
		metaffi_int32 i32 = 5;
		metaffi_uint32 ui32 = 6;
		metaffi_int64 i64 = 7;
		metaffi_uint64 ui64 = 8;
		metaffi_bool b = TRUE;
		metaffi_char8 c8;
		c8 = u8"א";
		metaffi_char16 c16;
		c16 = u"ב";
		metaffi_char32 c32(U'ג');	
		metaffi_string8 s8 = (metaffi_string8) u8"Hello UTF-8";
		metaffi_string16 s16 = (metaffi_string16) u"Hello UTF-16";
		metaffi_string32 s32 = (metaffi_string32) U"Hello UTF-32";
		cdt_metaffi_handle h((void*) 1, 101, nullptr);
		cdt_metaffi_callable callable((void*) 2, {metaffi_int8_type, metaffi_int16_type}, {metaffi_float32_type});

		std::vector<metaffi_variant> data = {f32, f64, i8, ui8, i16, ui16, i32, ui32, i64, ui64, b, c8, s8, c16, s16,
		                                     c32, s32, h, callable};
		
		cdts arr;

		construct_cdts_callbacks ccb{};
		ccb.context = (void*) &data;
		ccb.get_root_elements_count = [](void* context) -> metaffi_size {
			return ((std::vector<metaffi_variant>*) context)->size();
		};

		ccb.get_type_info = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_type_info {
			REQUIRE(index_length == 1);

			switch(index[0])
			{
				case 0:
					return {metaffi_float32_type, nullptr};
				case 1:
					return {metaffi_float64_type, nullptr};
				case 2:
					return {metaffi_int8_type, nullptr};
				case 3:
					return {metaffi_uint8_type, nullptr};
				case 4:
					return {metaffi_int16_type, nullptr};
				case 5:
					return {metaffi_uint16_type, nullptr};
				case 6:
					return {metaffi_int32_type, nullptr};
				case 7:
					return {metaffi_uint32_type, nullptr};
				case 8:
					return {metaffi_int64_type, nullptr};
				case 9:
					return {metaffi_uint64_type, nullptr};
				case 10:
					return {metaffi_bool_type, nullptr};
				case 11:
					return {metaffi_char8_type, nullptr};
				case 12:
					return {metaffi_string8_type, nullptr};
				case 13:
					return {metaffi_char16_type, nullptr};
				case 14:
					return {metaffi_string16_type, nullptr};
				case 15:
					return {metaffi_char32_type, nullptr};
				case 16:
					return {metaffi_string32_type, nullptr};
				case 17:
					return {metaffi_handle_type, nullptr};
				case 18:
					return {metaffi_callable_type, nullptr};
				default:
					throw std::runtime_error("Unexpected index");
			}
		};

		ccb.get_float32 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_float32 {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 0));
			REQUIRE((std::get<metaffi_float32>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 3.14f));
			return std::get<metaffi_float32>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_float64 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_float64 {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 1));
			REQUIRE((std::get<metaffi_float64>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 2.71828));
			return std::get<metaffi_float64>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_int8 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_int8 {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 2));
			REQUIRE((std::get<metaffi_int8>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 1));
			return std::get<metaffi_int8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_uint8 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_uint8 {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 3));
			REQUIRE((std::get<metaffi_uint8>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 2));
			return std::get<metaffi_uint8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_int16 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_int16 {
			REQUIRE((index_length == 1));
			REQUIRE(index[0] == 4);
			REQUIRE(std::get<metaffi_int16>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 3);
			return std::get<metaffi_int16>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_uint16 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_uint16 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 5);
			REQUIRE(std::get<metaffi_uint16>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 4);
			return std::get<metaffi_uint16>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_int32 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_int32 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 6);
			REQUIRE(std::get<metaffi_int32>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 5);
			return std::get<metaffi_int32>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_uint32 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_uint32 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 7);
			REQUIRE(std::get<metaffi_uint32>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 6);
			return std::get<metaffi_uint32>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_int64 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_int64 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 8);
			REQUIRE(std::get<metaffi_int64>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 7);
			return std::get<metaffi_int64>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_uint64 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_uint64 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 9);
			REQUIRE(std::get<metaffi_uint64>(((std::vector<metaffi_variant>*) context)->at(index[0])) == 8);
			return std::get<metaffi_uint64>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_bool = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_bool {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 10);
			REQUIRE(std::get<metaffi_bool>(((std::vector<metaffi_variant>*) context)->at(index[0])) == TRUE);
			return std::get<metaffi_bool>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_char8 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_char8 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 11);
			metaffi_char8 retchar = std::get<metaffi_char8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u8string mffi_u8char(reinterpret_cast<const char8_t*>(retchar.c));

			REQUIRE(mffi_u8char == u8"א");
			return std::get<metaffi_char8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_string8 = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_free_required, void* context) -> metaffi_string8 {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 12));
			metaffi_string8 retstr = std::get<metaffi_string8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u8string mffi_u8str(reinterpret_cast<const char8_t*>(retstr));
			*is_free_required = FALSE;

			REQUIRE((mffi_u8str == u8"Hello UTF-8"));
			return std::get<metaffi_string8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_char16 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_char16 {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 13));

			//maybe add == operator for metaffi_char16
			return std::get<metaffi_char16>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_string16 = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_free_required, void* context) -> metaffi_string16 {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 14));
			metaffi_string16 retstr = std::get<metaffi_string16>(
			        ((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u16string mffi_u16str(reinterpret_cast<const char16_t*>(retstr));
			*is_free_required = FALSE;

			REQUIRE((mffi_u16str == u"Hello UTF-16"));
			return std::get<metaffi_string16>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_char32 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_char32 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 15);
			REQUIRE(std::get<metaffi_char32>(((std::vector<metaffi_variant>*) context)->at(index[0])).c == U'ג');
			return std::get<metaffi_char32>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_string32 = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_free_required, void* context) -> metaffi_string32 {
			REQUIRE(index_length == 1);
			REQUIRE(index[0] == 16);
			metaffi_string32 retstr = std::get<metaffi_string32>(
			        ((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u32string mffi_u32str(reinterpret_cast<const char32_t*>(retstr));
			*is_free_required = FALSE;
			
			REQUIRE(mffi_u32str == U"Hello UTF-32");
			return std::get<metaffi_string32>(((std::vector<metaffi_variant>*) context)->at(index[0]));
		};

		ccb.get_handle = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_free_required, void* context) -> cdt_metaffi_handle* {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 17));
			REQUIRE((std::get<cdt_metaffi_handle>(((std::vector<metaffi_variant>*) context)->at(index[0])) ==
			        cdt_metaffi_handle((void*) 1, 101, nullptr)));
			*is_free_required = FALSE;
			return new cdt_metaffi_handle(std::get<cdt_metaffi_handle>(((std::vector<metaffi_variant>*) context)->at(index[0])));
		};

		ccb.get_callable = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_free_required, void* context) -> cdt_metaffi_callable* {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 18));
			cdt_metaffi_callable mcallable((void*) 2, {metaffi_int8_type, metaffi_int16_type}, {metaffi_float32_type});
			REQUIRE((std::get<cdt_metaffi_callable>(((std::vector<metaffi_variant>*) context)->at(index[0])) == mcallable));
			mcallable.free();
			*is_free_required = TRUE;
			
			return new cdt_metaffi_callable(std::get<cdt_metaffi_callable>(((std::vector<metaffi_variant>*) context)->at(index[0])));
		};

		// construct the CDTS
		construct_cdts(arr, ccb);

		// traverse the CDTS and check it is correct
		traverse_cdts_callbacks tcb;
		tcb.context = (void*) &data;
		tcb.on_float32 = [](const metaffi_size* index, metaffi_size index_size, metaffi_float32 val, void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 0));
			REQUIRE((val == 3.14f));
		};

		tcb.on_float64 = [](const metaffi_size* index, metaffi_size index_size, metaffi_float64 val, void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 1));
			REQUIRE((val == 2.71828));
		};

		tcb.on_int8 = [](const metaffi_size* index, metaffi_size index_size, metaffi_int8 val, void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 2));
			REQUIRE((val == 1));
		};

		tcb.on_uint8 = [](const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 3));
			REQUIRE((val == 2));
		};

		tcb.on_int16 = [](const metaffi_size* index, metaffi_size index_size, metaffi_int16 val, void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 4));
			REQUIRE((val == 3));
		};

		tcb.on_uint16 = [](const metaffi_size* index, metaffi_size index_size, metaffi_uint16 val, void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 5));
			REQUIRE((val == 4));
		};

		tcb.on_int32 = [](const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 6);
			REQUIRE(val == 5);
		};

		tcb.on_uint32 = [](const metaffi_size* index, metaffi_size index_size, metaffi_uint32 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 7);
			REQUIRE(val == 6);
		};

		tcb.on_int64 = [](const metaffi_size* index, metaffi_size index_size, metaffi_int64 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 8);
			REQUIRE(val == 7);
		};

		tcb.on_uint64 = [](const metaffi_size* index, metaffi_size index_size, metaffi_uint64 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 9);
			REQUIRE(val == 8);
		};

		tcb.on_bool = [](const metaffi_size* index, metaffi_size index_size, metaffi_bool val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 10);
			REQUIRE(val == TRUE);
		};

		tcb.on_char8 = [](const metaffi_size* index, metaffi_size index_size, metaffi_char8 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 11);
			metaffi_char8 retchar = std::get<metaffi_char8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u8string mffi_u8char(reinterpret_cast<const char8_t*>(retchar.c));

			REQUIRE(mffi_u8char == u8"א");
		};

		tcb.on_string8 = [](const metaffi_size* index, metaffi_size index_size, metaffi_string8 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 12);
			metaffi_string8 retstr = std::get<metaffi_string8>(((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u8string mffi_u8str(reinterpret_cast<const char8_t*>(retstr));

			REQUIRE(mffi_u8str == u8"Hello UTF-8");
		};

		tcb.on_char16 = [](const metaffi_size* index, metaffi_size index_size, metaffi_char16 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 13);
			metaffi_char16 retchar = std::get<metaffi_char16>(((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u16string mffi_u16char(reinterpret_cast<const char16_t*>(retchar.c));

			REQUIRE(mffi_u16char == u"ב");
		};

		tcb.on_string16 = [](const metaffi_size* index, metaffi_size index_size, metaffi_string16 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 14);
			metaffi_string16 retstr = std::get<metaffi_string16>(
			        ((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u16string mffi_u16str(reinterpret_cast<const char16_t*>(retstr));

			REQUIRE(mffi_u16str == u"Hello UTF-16");
		};

		tcb.on_char32 = [](const metaffi_size* index, metaffi_size index_size, metaffi_char32 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 15);
			REQUIRE(val.c == U'ג');
		};

		tcb.on_string32 = [](const metaffi_size* index, metaffi_size index_size, metaffi_string32 val, void* context) {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 16);
			metaffi_string32 retstr = std::get<metaffi_string32>(
			        ((std::vector<metaffi_variant>*) context)->at(index[0]));
			std::u32string mffi_u32str(reinterpret_cast<const char32_t*>(retstr));

			REQUIRE((mffi_u32str == U"Hello UTF-32"));
		};

		tcb.on_handle = [](const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_handle& val, void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 17));
			REQUIRE((val == cdt_metaffi_handle((void*) 1, 101, nullptr)));
		};

		tcb.on_callable = [](const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_callable& val,
		                     void* context) {
			REQUIRE((index_size == 1));
			REQUIRE((index[0] == 18));
			cdt_metaffi_callable mcall((void*) 2, {metaffi_int8_type, metaffi_int16_type}, {metaffi_float32_type});
			REQUIRE((val.val == mcall.val));
		};

		traverse_cdts(arr, tcb);
	}

	TEST_CASE("1D array")
	{
		// Initialize a 4D ragged C-array
		metaffi_uint8 c_array[] = {1, 2, 3, 4, 5, 6, 7, 8};

		// Construct cdt_metaffi_int32_array using construct_multidim_array_bfs
		cdts arr;
		
		construct_cdts_callbacks ccb = {};
		ccb.context = (void*) c_array;
		ccb.get_root_elements_count = [](void* context) -> metaffi_size {
			return 1;
		};

		ccb.get_array_metadata = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_fixed_dimension,
		                            metaffi_bool* is_1d_array, metaffi_type* common_type, metaffi_bool* is_manually_construct_array, void* context) -> metaffi_size {
			REQUIRE((index_length == 1));
			REQUIRE((index[0] == 0));
			REQUIRE((*common_type == metaffi_uint8_type));
			*common_type = metaffi_uint8_type;
			*is_fixed_dimension = 1;
			*is_1d_array = 1;
			return 8;
		};

		ccb.get_uint8 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_uint8 {
			REQUIRE((index_length == 2));
			REQUIRE((index[1] < 8));
			return ((metaffi_uint8*) context)[index[1]];
		};

		ccb.get_type_info = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_type_info {
			REQUIRE((index_length >= 1));
			REQUIRE((index_length <= 2));
			REQUIRE((index[0] == 0));
			if(index_length == 1)
			{
				return {metaffi_uint8_type | metaffi_array_type, nullptr};
			}
			else
			{
				return {metaffi_uint8_type, nullptr};
			}
		};

		construct_cdts(arr, ccb);

		traverse_cdts_callbacks tcb;
		tcb.context = (void*) c_array;
		tcb.on_array = [](const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context) -> metaffi_bool {
			REQUIRE(index_size == 1);
			REQUIRE(index[0] == 0);
			REQUIRE(fixed_dimensions == 1);
			REQUIRE(val.length == 8);
			REQUIRE(common_type == metaffi_uint8_type);
			return true;
		};

		tcb.on_uint8 = [](const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context) {
			REQUIRE(index_size >= 1);
			REQUIRE(index_size <= 2);
			REQUIRE(index[1] < 8);
			REQUIRE(val == ((metaffi_uint8*) context)[index[1]]);
		};

		traverse_cdts(arr, tcb);
	}
	//--------------------------------------------------------------------
	TEST_CASE("ragged array")
	{
		auto init_4d_ragged_c_array = []() -> metaffi_int32**** {
			int elem = 1;
			// Initialize a 4D ragged array with size 2 for each dimension
			metaffi_int32**** arr = new metaffi_int32***[1];
			for(int i = 0; i < 1; ++i)
			{
				arr[i] = new metaffi_int32**[2];
				for(int j = 0; j < 2; ++j)
				{
					arr[i][j] = new metaffi_int32*[3];
					for(int k = 0; k < 3; ++k)
					{
						arr[i][j][k] = new metaffi_int32[4];
						for(int l = 0; l < 4; ++l)
						{
							// Assign some values to the 1D arrays
							arr[i][j][k][l] = elem;
							elem++;
						}
					}
				}
			}
			return arr;
		};

		// Initialize a 4D ragged C-array
		metaffi_int32**** c_array = init_4d_ragged_c_array();
		metaffi::utils::scope_guard sg([&c_array]() {
			for(int i = 0; i < 1; ++i)
			{
				for(int j = 0; j < 2; ++j)
				{
					for(int k = 0; k < 3; ++k)
					{
						delete[] c_array[i][j][k];
					}
					delete[] c_array[i][j];
				}
				delete[] c_array[i];
			}
			delete[] c_array;
		});

		// Construct cdt_metaffi_int32_array using construct_multidim_array_bfs
		construct_cdts_callbacks ccb = {};
		ccb.context = (void*) c_array;

		ccb.get_root_elements_count = [](void* context) -> metaffi_size {
			return 1;
		};

		ccb.get_type_info = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_type_info {
			REQUIRE((index_length >= 1));
			REQUIRE((index_length <= 4));
			REQUIRE((index[0] == 0));
			if(index_length > 0 && index_length < 4)
			{
				return {metaffi_int32_type | metaffi_array_type, nullptr};
			}
			else
			{
				return {metaffi_int32_type, nullptr};
			}
		};

		ccb.get_array_metadata = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_fixed_dimension,
		                            metaffi_bool* is_1d_array, metaffi_type* common_type, metaffi_bool* is_manually_construct_array, void* context) -> metaffi_size {
			if(index_length == 1)
			{
				*is_fixed_dimension = true;
				*common_type = metaffi_int32_type;
				*is_1d_array = false;
				return index_length;
			}
			else if(index_length == 2)
			{
				*is_fixed_dimension = true;
				*common_type = metaffi_int32_type;
				*is_1d_array = false;
				return index_length;
			}
			else if(index_length == 3)
			{
				*is_fixed_dimension = true;
				*common_type = metaffi_int32_type;
				*is_1d_array = true;
				return index_length;
			}
			else
			{
				FAIL("Shouldn't reach here");
				return 0;
			}
		};

		ccb.get_int32 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_int32 {
			REQUIRE((index_length >= 1));
			REQUIRE((index_length <= 4));
			metaffi_int32**** cur = static_cast<metaffi_int32****>(context);
			for(metaffi_size i = 0; i < index_length; ++i)
			{
				cur = reinterpret_cast<metaffi_int32****>(cur[index[i]]);
			}
			
#pragma warning(push)
#pragma warning(disable: 4311)
			return static_cast<metaffi_int32>(reinterpret_cast<intptr_t>(cur));
#pragma warning(pop)
		};

		cdts arr;
		construct_cdts(arr, ccb);

		// Traverse the CDTS and check it is correct
		traverse_cdts_callbacks tcb{};
		tcb.context = (void*) c_array;

		tcb.on_array = [](const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context) -> metaffi_bool
		{
			REQUIRE(index_size >= 1);
			REQUIRE(index_size <= 4);
			REQUIRE(index[0] == 0);
			switch(index_size)
			{
				case 1: {
					REQUIRE(fixed_dimensions == 3);
					REQUIRE(common_type == metaffi_int32_type);
					REQUIRE(val.length == 1);
					break;
				}
				case 2: {
					REQUIRE(fixed_dimensions == 2);
					REQUIRE(common_type == metaffi_int32_type);
					REQUIRE(val.length == 2);
					break;
				}
				case 3: {
					REQUIRE(fixed_dimensions == 1);
					REQUIRE(common_type == metaffi_int32_type);
					REQUIRE(val.length == 3);
					break;
				}
				case 4: {
					REQUIRE(fixed_dimensions == 0);
					REQUIRE(common_type == metaffi_int32_type);
					REQUIRE(val.length == 4);
					break;
				}
				default: {
					FAIL("Shouldn't reach here");
				}
			}
			
			return true;
		};

		tcb.on_int32 = [](const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context) {
			REQUIRE((index_size >= 1));
			REQUIRE((index_size <= 4));
			metaffi_int32**** cur = static_cast<metaffi_int32****>(context);
			for(metaffi_size i = 0; i < index_size; ++i)
			{
				cur = reinterpret_cast<metaffi_int32****>(cur[index[i]]);
			}
#pragma warning(push)
#pragma warning(disable: 4311)
			auto expected = static_cast<metaffi_int32>(reinterpret_cast<intptr_t>(cur));
			REQUIRE((val == expected));
#pragma warning(pop)
		};

		traverse_cdts(arr, tcb);
		
	}

	TEST_CASE("mixed dimensions")
	{
		// create an array that looks like this: [1, [2,3]]

		using MixedArray = std::variant<metaffi_int32, std::vector<metaffi_int32>>;
		std::vector<MixedArray> arr;

		arr.emplace_back(1);
		arr.emplace_back(std::vector<metaffi_int32>{2, 3});

		construct_cdts_callbacks ccb = {};
		ccb.context = (void*) &arr;

		ccb.get_root_elements_count = [](void* context) -> metaffi_size {
			return 1;
		};

		ccb.get_type_info = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_type_info {
			if(index_length == 1)
			{
				REQUIRE(index[0] == 0);
				return {metaffi_int32_type | metaffi_array_type};
			}
			else if(index_length == 2)
			{
				if(index[1] == 0)
				{
					return {metaffi_int32_type};
				}
				else if(index[1] == 1)
				{
					return {metaffi_int32_type | metaffi_array_type, nullptr, false, 1};
				}
				else
				{
					FAIL("Shouldn't reach here");
				}
			}
			else if(index_length == 3)
			{
				REQUIRE(index[0] == 0);
				REQUIRE(index[1] == 1);
				return {metaffi_int32_type};
			}
			else
			{
				FAIL("Shouldn't reach here");
			}
			
			return {metaffi_null_type}; // to hide the warning...
		};

		ccb.get_array_metadata = [](const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_fixed_dimension, metaffi_bool* is_1d_array, metaffi_type* common_type, metaffi_bool* is_manually_construct_array, void* context) -> metaffi_size {
			REQUIRE(index[0] == 0);
			if(index_length == 1)
			{
				*is_fixed_dimension = false;
				*common_type = metaffi_int32_type;
				*is_1d_array = false;
				return 2;
			}
			else if(index_length == 2)
			{
				*is_fixed_dimension = 1;
				*common_type = metaffi_int32_type;
				*is_1d_array = 1;
				return 2;
			}
			else
			{
				FAIL("Shouldn't reach here");
				return 0;
			}
		};

		ccb.get_int32 = [](const metaffi_size* index, metaffi_size index_length, void* context) -> metaffi_int32 {
			if(index_length == 2)
			{
				REQUIRE(index[0] == 0);
				REQUIRE(index[1] == 0);
				return std::get<metaffi_int32>(((std::vector<MixedArray>*) context)->at(0));
			}
			else if(index_length == 3)
			{
				REQUIRE(index[0] == 0);
				REQUIRE(index[1] == 1);
				return std::get<std::vector<metaffi_int32>>(((std::vector<MixedArray>*) context)->at(index[1])).at(index[2]);
			}
			else
			{
				FAIL("Shouldn't reach here");
			}
			
			throw std::runtime_error("Shouldn't reach here");
		};

		cdts pcdts;
		construct_cdts(pcdts, ccb);

		traverse_cdts_callbacks tcb;

		tcb.context = (void*) &arr;
		tcb.on_array = [](const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions,
		                  metaffi_type common_type, void* context) -> metaffi_bool{
			if(index_size == 1)
			{
				REQUIRE(index[0] == 0);
			}
			else if(index_size == 2)
			{
				REQUIRE(index[0] == 0);
				REQUIRE(index[1] == 1);
				REQUIRE(common_type == metaffi_int32_type);
			}
			else
			{
				FAIL("Shouldn't reach here");
			}
			
			return true;
		};

		tcb.on_int32 = [](const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context) {
			if(index_size == 2)
			{
				REQUIRE(index[0] == 0);
				REQUIRE(index[1] == 0);
				REQUIRE(val == 1);
			}
			else if(index_size == 3)
			{
				REQUIRE(index[0] == 0);
				REQUIRE(index[1] == 1);
				if(index[2] == 0)
				{
					REQUIRE(val == 2);
				}
				else if(index[2] == 1)
				{
					REQUIRE(val == 3);
				}
				else
				{
					FAIL("Shouldn't reach here");
				}
			}
			else
			{
				FAIL("Shouldn't reach here");
			}
		};

		traverse_cdts(pcdts, tcb);
	}
}