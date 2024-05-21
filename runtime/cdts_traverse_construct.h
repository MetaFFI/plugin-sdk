#pragma once
#include "cdt.h"

#ifdef __cplusplus
namespace metaffi::runtime
{
#endif

//--------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
struct traverse_cdts_callbacks
{
	void* context;
	
	void (*on_float64)(const metaffi_size* index, metaffi_size index_size, metaffi_float64 val, void* context);
	void (*on_float32)(const metaffi_size* index, metaffi_size index_size, metaffi_float32 val, void* context);
	void (*on_int8)(const metaffi_size* index, metaffi_size index_size, metaffi_int8 val, void* context);
	void (*on_uint8)(const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context);
	void (*on_int16)(const metaffi_size* index, metaffi_size index_size, metaffi_int16 val, void* context);
	void (*on_uint16)(const metaffi_size* index, metaffi_size index_size, metaffi_uint16 val, void* context);
	void (*on_int32)(const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context);
	void (*on_uint32)(const metaffi_size* index, metaffi_size index_size, metaffi_uint32 val, void* context);
	void (*on_int64)(const metaffi_size* index, metaffi_size index_size, metaffi_int64 val, void* context);
	void (*on_uint64)(const metaffi_size* index, metaffi_size index_size, metaffi_uint64 val, void* context);
	void (*on_bool)(const metaffi_size* index, metaffi_size index_size, metaffi_bool val, void* context);
	void (*on_char8)(const metaffi_size* index, metaffi_size index_size, struct metaffi_char8 val, void* context);
	void (*on_string8)(const metaffi_size* index, metaffi_size index_size, metaffi_string8 val, void* context);
	void (*on_char16)(const metaffi_size* index, metaffi_size index_size, struct metaffi_char16 val, void* context);
	void (*on_string16)(const metaffi_size* index, metaffi_size index_size, metaffi_string16 val, void* context);
	void (*on_char32)(const metaffi_size* index, metaffi_size index_size, struct metaffi_char32 val, void* context);
	void (*on_string32)(const metaffi_size* index, metaffi_size index_size, metaffi_string32 val, void* context);
	#ifdef __cplusplus
	void (*on_handle)(const metaffi_size* index, metaffi_size index_size, const struct cdt_metaffi_handle& val, void* context);
	void (*on_callable)(const metaffi_size* index, metaffi_size index_size, const struct cdt_metaffi_callable& val, void* context);
	#else
	void (*on_handle)(const metaffi_size* index, metaffi_size index_size, const struct cdt_metaffi_handle* val, void* context);
	void (*on_callable)(const metaffi_size* index, metaffi_size index_size, const struct cdt_metaffi_callable* val, void* context);
	#endif
	void (*on_null)(const metaffi_size* index, metaffi_size index_size, void* context);
	
	// if array dimensions are fixed, fixed_dimensions is the dimensions count of the array
	// if array has a common type to all of its elements, common_type is the type of the elements. Otherwise, 0.
	#ifdef __cplusplus
	metaffi_bool (*on_array)(const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context);
	#else
	metaffi_bool (*on_array)(const metaffi_size* index, metaffi_size index_size, const struct cdts* val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context);
	#endif

#ifdef __cplusplus
	
	traverse_cdts_callbacks() = default;
	
	explicit traverse_cdts_callbacks(void* context) : context(context)
	{
		on_float64 = nullptr;
		on_float32 = nullptr;
		on_int8 = nullptr;
		on_uint8 = nullptr;
		on_int16 = nullptr;
		on_uint16 = nullptr;
		on_int32 = nullptr;
		on_uint32 = nullptr;
		on_int64 = nullptr;
		on_uint64 = nullptr;
		on_bool = nullptr;
		on_char8 = nullptr;
		on_string8 = nullptr;
		on_char16 = nullptr;
		on_string16 = nullptr;
		on_char32 = nullptr;
		on_string32 = nullptr;
		on_handle = nullptr;
		on_callable = nullptr;
		on_null = nullptr;
		on_array = nullptr;
	}
	
	traverse_cdts_callbacks(
			void* context,
			void (*on_float64)(const metaffi_size* index, metaffi_size index_size, metaffi_float64 val, void* context),
			void (*on_float32)(const metaffi_size* index, metaffi_size index_size, metaffi_float32 val, void* context),
			void (*on_int8)(const metaffi_size* index, metaffi_size index_size, metaffi_int8 val, void* context),
			void (*on_uint8)(const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context),
			void (*on_int16)(const metaffi_size* index, metaffi_size index_size, metaffi_int16 val, void* context),
			void (*on_uint16)(const metaffi_size* index, metaffi_size index_size, metaffi_uint16 val, void* context),
			void (*on_int32)(const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context),
			void (*on_uint32)(const metaffi_size* index, metaffi_size index_size, metaffi_uint32 val, void* context),
			void (*on_int64)(const metaffi_size* index, metaffi_size index_size, metaffi_int64 val, void* context),
			void (*on_uint64)(const metaffi_size* index, metaffi_size index_size, metaffi_uint64 val, void* context),
			void (*on_bool)(const metaffi_size* index, metaffi_size index_size, metaffi_bool val, void* context),
			void (*on_char8)(const metaffi_size* index, metaffi_size index_size, struct metaffi_char8 val, void* context),
			void (*on_string8)(const metaffi_size* index, metaffi_size index_size, metaffi_string8 val, void* context),
			void (*on_char16)(const metaffi_size* index, metaffi_size index_size, struct metaffi_char16 val, void* context),
			void (*on_string16)(const metaffi_size* index, metaffi_size index_size, metaffi_string16 val, void* context),
			void (*on_char32)(const metaffi_size* index, metaffi_size index_size, struct metaffi_char32 val, void* context),
			void (*on_string32)(const metaffi_size* index, metaffi_size index_size, metaffi_string32 val, void* context),
			void (*on_handle)(const metaffi_size* index, metaffi_size index_size, const struct cdt_metaffi_handle& val, void* context),
			void (*on_callable)(const metaffi_size* index, metaffi_size index_size, const struct cdt_metaffi_callable& val, void* context),
			void (*on_null)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_bool (*on_array)(const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context)
	) : context(context),
	    on_float64(on_float64),
	    on_float32(on_float32),
	    on_int8(on_int8),
	    on_uint8(on_uint8),
	    on_int16(on_int16),
	    on_uint16(on_uint16),
	    on_int32(on_int32),
	    on_uint32(on_uint32),
	    on_int64(on_int64),
	    on_uint64(on_uint64),
	    on_bool(on_bool),
	    on_char8(on_char8),
	    on_string8(on_string8),
	    on_char16(on_char16),
	    on_string16(on_string16),
	    on_char32(on_char32),
	    on_string32(on_string32),
	    on_handle(on_handle),
	    on_callable(on_callable),
	    on_null(on_null),
	    on_array(on_array)
	{}
#endif // __cplusplus
};
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
void traverse_cdts(const cdts& arr, const traverse_cdts_callbacks& callbacks);
extern "C" void traverse_cdts(const cdts* arr, const traverse_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept;
void traverse_cdts(const cdts& arr, const traverse_cdts_callbacks& callbacks, const std::vector<metaffi_size>& starting_index);
void traverse_cdt(const cdt& item, const traverse_cdts_callbacks& callbacks);
extern "C" void traverse_cdt(const cdt* item, const traverse_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept;
void traverse_cdt(const cdt& item, const traverse_cdts_callbacks& callbacks, const std::vector<metaffi_size>& current_index);
#else
void traverse_cdts(const struct cdts* arr, const struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err);
void traverse_cdt(const struct cdt* item, const struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err);
#endif // __cplusplus
//--------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
struct construct_cdts_callbacks
{
	void* context;
	
	// returns the size of the array at index.
	// if the array has fixed dimensions, set is_fixed_dimension to true.
	// if the array is 1D, set is_1d_array to true.
	// if the array has a common type to all of its elements, set common_type to the type of the elements.
	// if the array is manually constructed, set is_manually_construct_array to true. It will call construct_cdt_array to fill the array. Otherwise, the function iterates into every element and calls the appropriate callback.
	metaffi_size(*get_array_metadata)(const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_fixed_dimension, metaffi_bool* is_1d_array, metaffi_type* common_type, metaffi_bool* is_manually_construct_array, void* context);
	void(*construct_cdt_array)(const metaffi_size* index, metaffi_size index_length, struct cdts* manually_fill_array, void* context);
	
	metaffi_size (*get_root_elements_count)(void* context);
	struct metaffi_type_info (*get_type_info)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_float64 (*get_float64)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_float32 (*get_float32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int8 (*get_int8)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint8 (*get_uint8)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int16 (*get_int16)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint16 (*get_uint16)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int32 (*get_int32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint32 (*get_uint32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int64 (*get_int64)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint64 (*get_uint64)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_bool (*get_bool)(const metaffi_size* index, metaffi_size index_size, void* context);
	struct metaffi_char8 (*get_char8)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_string8 (*get_string8)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context);
	struct metaffi_char16 (*get_char16)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_string16 (*get_string16)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context);
	struct metaffi_char32 (*get_char32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_string32 (*get_string32)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context);
	struct cdt_metaffi_handle* (*get_handle)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context);
	struct cdt_metaffi_callable* (*get_callable)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context);

#ifdef __cplusplus
	
	construct_cdts_callbacks() = default;
	
	explicit construct_cdts_callbacks(void* context):context(context)
	{
		get_array_metadata = nullptr;
		construct_cdt_array = nullptr;
		get_root_elements_count = nullptr;
		get_type_info = nullptr;
		get_float64 = nullptr;
		get_float32 = nullptr;
		get_int8 = nullptr;
		get_uint8 = nullptr;
		get_int16 = nullptr;
		get_uint16 = nullptr;
		get_int32 = nullptr;
		get_uint32 = nullptr;
		get_int64 = nullptr;
		get_uint64 = nullptr;
		get_bool = nullptr;
		get_char8 = nullptr;
		get_string8 = nullptr;
		get_char16 = nullptr;
		get_string16 = nullptr;
		get_char32 = nullptr;
		get_string32 = nullptr;
		get_handle = nullptr;
		get_callable = nullptr;
	}
	
	construct_cdts_callbacks(
			void* context,
		    metaffi_size(*get_array_metadata)(const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_fixed_dimension, metaffi_bool* is_1d_array, metaffi_type* common_type, metaffi_bool* is_manually_construct_array, void* context),
		    void(*construct_cdt_array)(const metaffi_size* index, metaffi_size index_length, cdts* manually_fill_array, void* context),
			metaffi_size (*get_length)(void* context),
			metaffi_type_info (*get_type_info)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_float64 (*get_float64)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_float32 (*get_float32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int8 (*get_int8)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint8 (*get_uint8)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int16 (*get_int16)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint16 (*get_uint16)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int32 (*get_int32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint32 (*get_uint32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int64 (*get_int64)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint64 (*get_uint64)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_bool (*get_bool)(const metaffi_size* index, metaffi_size index_size, void* context),
			struct metaffi_char8 (*get_char8)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_string8 (*get_string8)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context),
			struct metaffi_char16 (*get_char16)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_string16 (*get_string16)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context),
			struct metaffi_char32 (*get_char32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_string32 (*get_string32)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context),
			struct cdt_metaffi_handle* (*get_handle)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context),
			struct cdt_metaffi_callable* (*get_callable)(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
	) : context(context),
	    get_array_metadata(get_array_metadata),
        construct_cdt_array(construct_cdt_array),
	    get_root_elements_count(get_length),
	    get_type_info(get_type_info),
	    get_float64(get_float64),
	    get_float32(get_float32),
	    get_int8(get_int8),
	    get_uint8(get_uint8),
	    get_int16(get_int16),
	    get_uint16(get_uint16),
	    get_int32(get_int32),
	    get_uint32(get_uint32),
	    get_int64(get_int64),
	    get_uint64(get_uint64),
	    get_bool(get_bool),
	    get_char8(get_char8),
	    get_string8(get_string8),
	    get_char16(get_char16),
	    get_string16(get_string16),
	    get_char32(get_char32),
	    get_string32(get_string32),
	    get_handle(get_handle),
	    get_callable(get_callable)
	{}
	
#endif // __cplusplus
};
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
void construct_cdts(cdts& arr, const construct_cdts_callbacks& callbacks);
extern "C" void construct_cdts(cdts* arr, const construct_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept;
void construct_cdts(cdts& arr, const construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& starting_index, const metaffi_type_info& known_type = metaffi_type_info{metaffi_any_type});
void construct_cdt(cdt& item, const construct_cdts_callbacks& callbacks);
extern "C" void construct_cdt(cdt* item, const construct_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept;
void construct_cdt(cdt& item, const construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& current_index, const metaffi_type_info& known_type = metaffi_type_info{metaffi_any_type});
#else
void construct_cdts(struct cdts* arr, const struct construct_cdts_callbacks* callbacks, char** out_nul_term_err);
void construct_cdt(struct cdt* item, const struct construct_cdts_callbacks* callbacks, char** out_nul_term_err);
#endif // __cplusplus
//--------------------------------------------------------------------

#ifdef __cplusplus
} // namespace metaffi::runtime
#endif