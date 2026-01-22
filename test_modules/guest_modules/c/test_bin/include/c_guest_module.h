#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#	ifdef C_GUEST_MODULE_EXPORTS
#		define C_GUEST_API __declspec(dllexport)
#	else
#		define C_GUEST_API __declspec(dllimport)
#	endif
#else
#	define C_GUEST_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*c_add_callback)(int a, int b);
typedef const char* (*c_string_callback)(const char* value);
typedef void (*c_void_callback)(void);

typedef struct CGuestSomeClass {
	char* name;
} CGuestSomeClass;

typedef struct CGuestTestMap CGuestTestMap;

typedef struct CGuestPlainStruct {
	int id;
	const char* name;
} CGuestPlainStruct;

typedef enum CGuestColor {
	C_GUEST_COLOR_RED = 0,
	C_GUEST_COLOR_GREEN = 1,
	C_GUEST_COLOR_BLUE = 2
} CGuestColor;

typedef struct CGuestVec2 {
	int x;
	int y;
} CGuestVec2;

typedef struct CGuestMultiReturn {
	int64_t i;
	const char* s;
	double d;
	int is_null;
	const uint8_t* bytes;
	size_t bytes_len;
	CGuestSomeClass obj;
} CGuestMultiReturn;

typedef struct CGuestIntArray {
	int* data;
	size_t len;
} CGuestIntArray;

typedef struct CGuestInt2DArray {
	CGuestIntArray* rows;
	size_t len;
} CGuestInt2DArray;

typedef struct CGuestInt3DArray {
	CGuestInt2DArray* planes;
	size_t len;
} CGuestInt3DArray;

typedef struct CGuestByteArray {
	uint8_t* data;
	size_t len;
} CGuestByteArray;

typedef struct CGuestByte2DArray {
	CGuestByteArray* rows;
	size_t len;
} CGuestByte2DArray;

typedef struct CGuestStringArray {
	const char** data;
	size_t len;
} CGuestStringArray;

typedef struct CGuestStringIntPair {
	const char* key;
	int value;
} CGuestStringIntPair;

typedef struct CGuestStringIntMap {
	CGuestStringIntPair* items;
	size_t len;
} CGuestStringIntMap;

typedef struct CGuestAny {
	int type;
	union {
		int64_t i64;
		double f64;
		const char* str;
		CGuestSomeClass obj;
		CGuestByteArray bytes;
	} val;
} CGuestAny;

C_GUEST_API extern const int64_t C_GUEST_CONST_FIVE_SECONDS;
C_GUEST_API extern int64_t c_guest_counter;

C_GUEST_API int64_t c_guest_get_counter(void);
C_GUEST_API void c_guest_set_counter(int64_t value);
C_GUEST_API int64_t c_guest_inc_counter(int64_t delta);

C_GUEST_API void c_guest_hello_world(void);
C_GUEST_API int c_guest_returns_an_error(void);
C_GUEST_API double c_guest_div_integers(int64_t x, int64_t y);
C_GUEST_API const char* c_guest_join_strings(CGuestStringArray arr);
C_GUEST_API void c_guest_wait_a_bit(int64_t ms);
C_GUEST_API void* c_guest_return_null(void);

C_GUEST_API int c_guest_call_callback_add(c_add_callback cb);
C_GUEST_API c_add_callback c_guest_return_callback_add(void);
C_GUEST_API const char* c_guest_call_callback_string(c_string_callback cb, const char* value);
C_GUEST_API const char* c_guest_call_callback_with_error(c_void_callback cb);

C_GUEST_API CGuestSomeClass c_guest_someclass_create(const char* name);
C_GUEST_API void c_guest_someclass_destroy(CGuestSomeClass* obj);
C_GUEST_API const char* c_guest_someclass_print(const CGuestSomeClass* obj);
C_GUEST_API CGuestSomeClass c_guest_someclass_make_default(void);

C_GUEST_API CGuestTestMap* c_guest_testmap_create(void);
C_GUEST_API void c_guest_testmap_destroy(CGuestTestMap* map);
C_GUEST_API void c_guest_testmap_set(CGuestTestMap* map, const char* key, CGuestAny value);
C_GUEST_API CGuestAny c_guest_testmap_get(CGuestTestMap* map, const char* key);
C_GUEST_API int c_guest_testmap_contains(CGuestTestMap* map, const char* key);
C_GUEST_API const char* c_guest_testmap_get_name(CGuestTestMap* map);
C_GUEST_API void c_guest_testmap_set_name(CGuestTestMap* map, const char* name);

C_GUEST_API CGuestSomeClass* c_guest_get_some_classes(size_t* out_len);
C_GUEST_API void c_guest_expect_three_some_classes(const CGuestSomeClass* arr, size_t len);
C_GUEST_API void c_guest_free_some_classes(CGuestSomeClass* arr, size_t len);

C_GUEST_API CGuestByte2DArray c_guest_get_three_buffers(void);
C_GUEST_API int c_guest_expect_three_buffers(CGuestByte2DArray buffers);
C_GUEST_API void c_guest_free_byte2d(CGuestByte2DArray buffers);

C_GUEST_API CGuestIntArray c_guest_make_1d_array(void);
C_GUEST_API CGuestInt2DArray c_guest_make_2d_array(void);
C_GUEST_API CGuestInt3DArray c_guest_make_3d_array(void);
C_GUEST_API CGuestInt2DArray c_guest_make_ragged_array(void);
C_GUEST_API int c_guest_sum_3d_array(CGuestInt3DArray arr);
C_GUEST_API int c_guest_sum_ragged_array(CGuestInt2DArray arr);
C_GUEST_API void c_guest_free_int_array(CGuestIntArray arr);
C_GUEST_API void c_guest_free_int2d(CGuestInt2DArray arr);
C_GUEST_API void c_guest_free_int3d(CGuestInt3DArray arr);

C_GUEST_API CGuestAny* c_guest_returns_array_with_different_dimensions(size_t* out_len);
C_GUEST_API CGuestAny* c_guest_returns_array_of_different_objects(size_t* out_len);
C_GUEST_API void c_guest_free_any_array(CGuestAny* arr, size_t len);

C_GUEST_API CGuestAny c_guest_return_any(int which);
C_GUEST_API int c_guest_accepts_any(int which, CGuestAny value);

C_GUEST_API CGuestAny c_guest_accepts_primitives(bool b, int8_t i8, int16_t i16, int32_t i32, int64_t i64,
	uint8_t u8, uint16_t u16, uint32_t u32, uint64_t u64, float f32, double f64, char c);

C_GUEST_API CGuestByteArray c_guest_echo_bytes(CGuestByteArray data);
C_GUEST_API CGuestStringIntMap c_guest_make_string_int_map(void);
C_GUEST_API void c_guest_free_string_int_map(CGuestStringIntMap map);

C_GUEST_API CGuestMultiReturn c_guest_return_multiple_return_values(void);
C_GUEST_API int c_guest_return_error_pair(bool ok, const char** out_error);

C_GUEST_API CGuestVec2 c_guest_add_vec2(CGuestVec2 a, CGuestVec2 b);
C_GUEST_API int c_guest_vec2_equals(CGuestVec2 a, CGuestVec2 b);

C_GUEST_API CGuestColor c_guest_get_color(int idx);
C_GUEST_API const char* c_guest_color_name(CGuestColor color);

C_GUEST_API int c_guest_add_int(int a, int b);
C_GUEST_API double c_guest_add_double(double a, double b);
C_GUEST_API const char* c_guest_add_string(const char* a, const char* b);
C_GUEST_API int c_guest_default_args(int a, int b);
C_GUEST_API int c_guest_sum_variadic(int count, ...);
C_GUEST_API const char* c_guest_join_variadic(const char* prefix, int count, ...);

C_GUEST_API CGuestPlainStruct c_guest_make_plain_struct(int id, const char* name);

#ifdef __cplusplus
}
#endif
