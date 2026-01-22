#include "c_guest_module.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int64_t C_GUEST_CONST_FIVE_SECONDS = 5;
int64_t c_guest_counter = 0;

int64_t c_guest_get_counter(void) {
	return c_guest_counter;
}

void c_guest_set_counter(int64_t value) {
	c_guest_counter = value;
}

int64_t c_guest_inc_counter(int64_t delta) {
	c_guest_counter += delta;
	return c_guest_counter;
}

void c_guest_hello_world(void) {
	++c_guest_counter;
	printf("Hello World, from C\n");
}

int c_guest_returns_an_error(void) {
	return -1;
}

double c_guest_div_integers(int64_t x, int64_t y) {
	return (double)x / (double)y;
}

const char* c_guest_join_strings(CGuestStringArray arr) {
	size_t total = 1;
	for (size_t i = 0; i < arr.len; ++i) {
		total += strlen(arr.data[i]) + 1;
	}
	char* out = (char*)malloc(total);
	if (!out) {
		return NULL;
	}
	out[0] = '\0';
	for (size_t i = 0; i < arr.len; ++i) {
		strcat(out, arr.data[i]);
		if (i + 1 < arr.len) {
			strcat(out, ",");
		}
	}
	return out;
}

void c_guest_wait_a_bit(int64_t ms) {
	(void)ms;
}

void* c_guest_return_null(void) {
	return NULL;
}

static int c_guest_add_impl(int a, int b) {
	return a + b;
}

int c_guest_call_callback_add(c_add_callback cb) {
	int res = cb(1, 2);
	return res == 3 ? 0 : -1;
}

c_add_callback c_guest_return_callback_add(void) {
	return &c_guest_add_impl;
}

const char* c_guest_call_callback_string(c_string_callback cb, const char* value) {
	return cb(value);
}

const char* c_guest_call_callback_with_error(c_void_callback cb) {
	if (!cb) {
		return "callback is null";
	}
	cb();
	return "";
}

CGuestSomeClass c_guest_someclass_create(const char* name) {
	CGuestSomeClass obj;
	obj.name = NULL;
	if (name) {
		size_t len = strlen(name) + 1;
		obj.name = (char*)malloc(len);
		if (obj.name) {
			memcpy(obj.name, name, len);
		}
	}
	return obj;
}

void c_guest_someclass_destroy(CGuestSomeClass* obj) {
	if (!obj) {
		return;
	}
	free(obj->name);
	obj->name = NULL;
}

const char* c_guest_someclass_print(const CGuestSomeClass* obj) {
	if (!obj || !obj->name) {
		return "Hello from SomeClass";
	}
	size_t len = strlen(obj->name) + 22;
	char* out = (char*)malloc(len);
	if (!out) {
		return NULL;
	}
	snprintf(out, len, "Hello from SomeClass %s", obj->name);
	return out;
}

CGuestSomeClass c_guest_someclass_make_default(void) {
	return c_guest_someclass_create("some");
}

struct CGuestTestMap {
	CGuestStringIntPair* items;
	size_t len;
	char* name;
};

CGuestTestMap* c_guest_testmap_create(void) {
	CGuestTestMap* map = (CGuestTestMap*)calloc(1, sizeof(CGuestTestMap));
	if (!map) {
		return NULL;
	}
	map->name = strdup("name1");
	return map;
}

void c_guest_testmap_destroy(CGuestTestMap* map) {
	if (!map) {
		return;
	}
	for (size_t i = 0; i < map->len; ++i) {
		free((void*)map->items[i].key);
	}
	free(map->items);
	free(map->name);
	free(map);
}

void c_guest_testmap_set(CGuestTestMap* map, const char* key, CGuestAny value) {
	if (!map || !key) {
		return;
	}
	for (size_t i = 0; i < map->len; ++i) {
		if (strcmp(map->items[i].key, key) == 0) {
			map->items[i].value = (int)value.val.i64;
			return;
		}
	}
	CGuestStringIntPair* new_items = (CGuestStringIntPair*)realloc(map->items, sizeof(CGuestStringIntPair) * (map->len + 1));
	if (!new_items) {
		return;
	}
	map->items = new_items;
	map->items[map->len].key = strdup(key);
	map->items[map->len].value = (int)value.val.i64;
	map->len += 1;
}

CGuestAny c_guest_testmap_get(CGuestTestMap* map, const char* key) {
	CGuestAny out = {0};
	if (!map || !key) {
		return out;
	}
	for (size_t i = 0; i < map->len; ++i) {
		if (strcmp(map->items[i].key, key) == 0) {
			out.type = 0;
			out.val.i64 = map->items[i].value;
			return out;
		}
	}
	return out;
}

int c_guest_testmap_contains(CGuestTestMap* map, const char* key) {
	if (!map || !key) {
		return 0;
	}
	for (size_t i = 0; i < map->len; ++i) {
		if (strcmp(map->items[i].key, key) == 0) {
			return 1;
		}
	}
	return 0;
}

const char* c_guest_testmap_get_name(CGuestTestMap* map) {
	return map ? map->name : NULL;
}

void c_guest_testmap_set_name(CGuestTestMap* map, const char* name) {
	if (!map) {
		return;
	}
	free(map->name);
	map->name = name ? strdup(name) : NULL;
}

CGuestSomeClass* c_guest_get_some_classes(size_t* out_len) {
	if (out_len) {
		*out_len = 3;
	}
	CGuestSomeClass* arr = (CGuestSomeClass*)calloc(3, sizeof(CGuestSomeClass));
	if (!arr) {
		return NULL;
	}
	arr[0] = c_guest_someclass_create("a");
	arr[1] = c_guest_someclass_create("b");
	arr[2] = c_guest_someclass_create("c");
	return arr;
}

void c_guest_expect_three_some_classes(const CGuestSomeClass* arr, size_t len) {
	if (!arr || len != 3) {
		return;
	}
}

void c_guest_free_some_classes(CGuestSomeClass* arr, size_t len) {
	if (!arr) {
		return;
	}
	for (size_t i = 0; i < len; ++i) {
		c_guest_someclass_destroy(&arr[i]);
	}
	free(arr);
}

CGuestByte2DArray c_guest_get_three_buffers(void) {
	CGuestByte2DArray out = {0};
	out.len = 3;
	out.rows = (CGuestByteArray*)calloc(out.len, sizeof(CGuestByteArray));
	if (!out.rows) {
		out.len = 0;
		return out;
	}
	uint8_t data1[] = {1, 2, 3, 4};
	uint8_t data2[] = {5, 6, 7};
	uint8_t data3[] = {8, 9};
	out.rows[0].len = sizeof(data1);
	out.rows[1].len = sizeof(data2);
	out.rows[2].len = sizeof(data3);
	out.rows[0].data = (uint8_t*)malloc(out.rows[0].len);
	out.rows[1].data = (uint8_t*)malloc(out.rows[1].len);
	out.rows[2].data = (uint8_t*)malloc(out.rows[2].len);
	memcpy(out.rows[0].data, data1, out.rows[0].len);
	memcpy(out.rows[1].data, data2, out.rows[1].len);
	memcpy(out.rows[2].data, data3, out.rows[2].len);
	return out;
}

int c_guest_expect_three_buffers(CGuestByte2DArray buffers) {
	if (buffers.len != 3) {
		return -1;
	}
	if (buffers.rows[0].len != 4 || buffers.rows[1].len != 3 || buffers.rows[2].len != 2) {
		return -1;
	}
	return 0;
}

void c_guest_free_byte2d(CGuestByte2DArray buffers) {
	for (size_t i = 0; i < buffers.len; ++i) {
		free(buffers.rows[i].data);
	}
	free(buffers.rows);
}

CGuestIntArray c_guest_make_1d_array(void) {
	CGuestIntArray arr = {0};
	arr.len = 3;
	arr.data = (int*)malloc(sizeof(int) * arr.len);
	if (arr.data) {
		arr.data[0] = 1;
		arr.data[1] = 2;
		arr.data[2] = 3;
	}
	return arr;
}

CGuestInt2DArray c_guest_make_2d_array(void) {
	CGuestInt2DArray arr = {0};
	arr.len = 2;
	arr.rows = (CGuestIntArray*)calloc(arr.len, sizeof(CGuestIntArray));
	if (!arr.rows) {
		arr.len = 0;
		return arr;
	}
	arr.rows[0] = c_guest_make_1d_array();
	arr.rows[1].len = 2;
	arr.rows[1].data = (int*)malloc(sizeof(int) * 2);
	if (arr.rows[1].data) {
		arr.rows[1].data[0] = 3;
		arr.rows[1].data[1] = 4;
	}
	return arr;
}

CGuestInt3DArray c_guest_make_3d_array(void) {
	CGuestInt3DArray arr = {0};
	arr.len = 2;
	arr.planes = (CGuestInt2DArray*)calloc(arr.len, sizeof(CGuestInt2DArray));
	if (!arr.planes) {
		arr.len = 0;
		return arr;
	}
	arr.planes[0] = c_guest_make_2d_array();
	arr.planes[1] = c_guest_make_2d_array();
	return arr;
}

CGuestInt2DArray c_guest_make_ragged_array(void) {
	CGuestInt2DArray arr = {0};
	arr.len = 3;
	arr.rows = (CGuestIntArray*)calloc(arr.len, sizeof(CGuestIntArray));
	if (!arr.rows) {
		arr.len = 0;
		return arr;
	}
	arr.rows[0].len = 3;
	arr.rows[0].data = (int*)malloc(sizeof(int) * 3);
	arr.rows[1].len = 1;
	arr.rows[1].data = (int*)malloc(sizeof(int));
	arr.rows[2].len = 2;
	arr.rows[2].data = (int*)malloc(sizeof(int) * 2);
	if (arr.rows[0].data) {
		arr.rows[0].data[0] = 1;
		arr.rows[0].data[1] = 2;
		arr.rows[0].data[2] = 3;
	}
	if (arr.rows[1].data) {
		arr.rows[1].data[0] = 4;
	}
	if (arr.rows[2].data) {
		arr.rows[2].data[0] = 5;
		arr.rows[2].data[1] = 6;
	}
	return arr;
}

int c_guest_sum_3d_array(CGuestInt3DArray arr) {
	int sum = 0;
	for (size_t i = 0; i < arr.len; ++i) {
		sum += c_guest_sum_ragged_array(arr.planes[i]);
	}
	return sum;
}

int c_guest_sum_ragged_array(CGuestInt2DArray arr) {
	int sum = 0;
	for (size_t i = 0; i < arr.len; ++i) {
		for (size_t j = 0; j < arr.rows[i].len; ++j) {
			sum += arr.rows[i].data[j];
		}
	}
	return sum;
}

void c_guest_free_int_array(CGuestIntArray arr) {
	free(arr.data);
}

void c_guest_free_int2d(CGuestInt2DArray arr) {
	for (size_t i = 0; i < arr.len; ++i) {
		free(arr.rows[i].data);
	}
	free(arr.rows);
}

void c_guest_free_int3d(CGuestInt3DArray arr) {
	for (size_t i = 0; i < arr.len; ++i) {
		c_guest_free_int2d(arr.planes[i]);
	}
	free(arr.planes);
}

CGuestAny* c_guest_returns_array_with_different_dimensions(size_t* out_len) {
	if (out_len) {
		*out_len = 3;
	}
	CGuestAny* arr = (CGuestAny*)calloc(3, sizeof(CGuestAny));
	if (!arr) {
		return NULL;
	}
	CGuestIntArray arr1 = c_guest_make_1d_array();
	arr[0].type = 100;
	arr[0].val.bytes.data = (uint8_t*)arr1.data;
	arr[0].val.bytes.len = arr1.len * sizeof(int);
	arr[1].type = 1;
	arr[1].val.i64 = 4;
	CGuestInt2DArray arr2 = c_guest_make_2d_array();
	arr[2].type = 101;
	arr[2].val.bytes.data = (uint8_t*)arr2.rows;
	arr[2].val.bytes.len = arr2.len;
	return arr;
}

CGuestAny* c_guest_returns_array_of_different_objects(size_t* out_len) {
	if (out_len) {
		*out_len = 6;
	}
	CGuestAny* arr = (CGuestAny*)calloc(6, sizeof(CGuestAny));
	if (!arr) {
		return NULL;
	}
	arr[0].type = 0;
	arr[0].val.i64 = 1;
	arr[1].type = 2;
	arr[1].val.str = "string";
	arr[2].type = 1;
	arr[2].val.f64 = 3.0;
	arr[3].type = 3;
	arr[3].val.str = NULL;
	arr[4].type = 4;
	arr[4].val.bytes = c_guest_echo_bytes((CGuestByteArray){(uint8_t*)"abc", 3});
	arr[5].type = 5;
	arr[5].val.obj = c_guest_someclass_create("x");
	return arr;
}

void c_guest_free_any_array(CGuestAny* arr, size_t len) {
	if (!arr) {
		return;
	}
	for (size_t i = 0; i < len; ++i) {
		if (arr[i].type == 5) {
			c_guest_someclass_destroy(&arr[i].val.obj);
		}
	}
	free(arr);
}

CGuestAny c_guest_return_any(int which) {
	CGuestAny out = {0};
	switch (which) {
		case 0:
			out.type = 0;
			out.val.i64 = 1;
			break;
		case 1:
			out.type = 2;
			out.val.str = "string";
			break;
		case 2:
			out.type = 1;
			out.val.f64 = 3.0;
			break;
		case 3:
			out.type = 3;
			out.val.str = NULL;
			break;
		case 4:
			out.type = 5;
			out.val.obj = c_guest_someclass_create("some");
			break;
		default:
			break;
	}
	return out;
}

int c_guest_accepts_any(int which, CGuestAny value) {
	switch (which) {
		case 0:
			return value.type == 0 ? 0 : -1;
		case 1:
			return value.type == 2 ? 0 : -1;
		case 2:
			return value.type == 1 ? 0 : -1;
		case 3:
			return value.type == 3 ? 0 : -1;
		case 4:
			return value.type == 4 ? 0 : -1;
		case 5:
			return value.type == 5 ? 0 : -1;
		default:
			return -1;
	}
}

CGuestAny c_guest_accepts_primitives(bool b, int8_t i8, int16_t i16, int32_t i32, int64_t i64,
	uint8_t u8, uint16_t u16, uint32_t u32, uint64_t u64, float f32, double f64, char c) {
	CGuestAny out = {0};
	out.type = 99;
	out.val.i64 = (int64_t)(b + i8 + i16 + i32 + i64 + u8 + u16 + u32 + u64 + (int64_t)f32 + (int64_t)f64 + c);
	return out;
}

CGuestByteArray c_guest_echo_bytes(CGuestByteArray data) {
	CGuestByteArray out = {0};
	out.len = data.len;
	out.data = (uint8_t*)malloc(out.len);
	if (out.data) {
		memcpy(out.data, data.data, out.len);
	}
	return out;
}

CGuestStringIntMap c_guest_make_string_int_map(void) {
	CGuestStringIntMap map = {0};
	map.len = 2;
	map.items = (CGuestStringIntPair*)calloc(map.len, sizeof(CGuestStringIntPair));
	if (!map.items) {
		map.len = 0;
		return map;
	}
	map.items[0].key = strdup("a");
	map.items[0].value = 1;
	map.items[1].key = strdup("b");
	map.items[1].value = 2;
	return map;
}

void c_guest_free_string_int_map(CGuestStringIntMap map) {
	for (size_t i = 0; i < map.len; ++i) {
		free((void*)map.items[i].key);
	}
	free(map.items);
}

CGuestMultiReturn c_guest_return_multiple_return_values(void) {
	CGuestMultiReturn out;
	out.i = 1;
	out.s = "string";
	out.d = 3.0;
	out.is_null = 1;
	static const uint8_t bytes[] = {1, 2, 3};
	out.bytes = bytes;
	out.bytes_len = sizeof(bytes);
	out.obj = c_guest_someclass_create("x");
	return out;
}

int c_guest_return_error_pair(bool ok, const char** out_error) {
	if (ok) {
		if (out_error) {
			*out_error = NULL;
		}
		return 0;
	}
	if (out_error) {
		*out_error = "error";
	}
	return -1;
}

CGuestVec2 c_guest_add_vec2(CGuestVec2 a, CGuestVec2 b) {
	CGuestVec2 out = {a.x + b.x, a.y + b.y};
	return out;
}

int c_guest_vec2_equals(CGuestVec2 a, CGuestVec2 b) {
	return a.x == b.x && a.y == b.y;
}

CGuestColor c_guest_get_color(int idx) {
	if (idx == 0) {
		return C_GUEST_COLOR_RED;
	}
	if (idx == 1) {
		return C_GUEST_COLOR_GREEN;
	}
	return C_GUEST_COLOR_BLUE;
}

const char* c_guest_color_name(CGuestColor color) {
	switch (color) {
		case C_GUEST_COLOR_RED:
			return "RED";
		case C_GUEST_COLOR_GREEN:
			return "GREEN";
		default:
			return "BLUE";
	}
}

int c_guest_add_int(int a, int b) {
	return a + b;
}

double c_guest_add_double(double a, double b) {
	return a + b;
}

const char* c_guest_add_string(const char* a, const char* b) {
	size_t len = strlen(a) + strlen(b) + 1;
	char* out = (char*)malloc(len);
	if (!out) {
		return NULL;
	}
	snprintf(out, len, "%s%s", a, b);
	return out;
}

int c_guest_default_args(int a, int b) {
	return a + b;
}

int c_guest_sum_variadic(int count, ...) {
	va_list args;
	va_start(args, count);
	int sum = 0;
	for (int i = 0; i < count; ++i) {
		sum += va_arg(args, int);
	}
	va_end(args);
	return sum;
}

const char* c_guest_join_variadic(const char* prefix, int count, ...) {
	va_list args;
	va_start(args, count);
	size_t total = strlen(prefix) + 1;
	for (int i = 0; i < count; ++i) {
		const char* val = va_arg(args, const char*);
		total += strlen(val) + 1;
	}
	va_end(args);
	char* out = (char*)malloc(total);
	if (!out) {
		return NULL;
	}
	strcpy(out, prefix);
	va_start(args, count);
	for (int i = 0; i < count; ++i) {
		const char* val = va_arg(args, const char*);
		strcat(out, ":");
		strcat(out, val);
	}
	va_end(args);
	return out;
}

CGuestPlainStruct c_guest_make_plain_struct(int id, const char* name) {
	CGuestPlainStruct out;
	out.id = id;
	out.name = name;
	return out;
}
