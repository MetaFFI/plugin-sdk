#ifndef METAFFI_PRIMITIVES_H
#define METAFFI_PRIMITIVES_H
#pragma once

#include <stdint.h>
#include <uchar.h>

#ifdef __cplusplus
#include <variant>
#include <vector>
#include <stdexcept>
#include <cstring>
#endif


#if __STDC_VERSION__ < 202302L && !__cplusplus
	typedef uint8_t char8_t;
#endif

typedef double metaffi_float64;
typedef float metaffi_float32;

typedef int8_t metaffi_int8;
typedef int16_t metaffi_int16;
typedef int32_t metaffi_int32;
typedef int64_t metaffi_int64;

typedef uint8_t metaffi_uint8;
typedef uint16_t metaffi_uint16;
typedef uint32_t metaffi_uint32;
typedef uint64_t metaffi_uint64;

typedef uint8_t metaffi_bool;

struct metaffi_char16;
struct metaffi_char32;


struct metaffi_char8
{
	char8_t c[4];

#ifdef __cplusplus
	static int8_t num_of_bytes(const char8_t* c)
	{
		auto firstByte = static_cast<uint8_t>(c[0]);
		
		return (firstByte & 0x80) == 0 ? 1:
			   (firstByte & 0xE0) == 0xC0 ? 2:
			   (firstByte & 0xF0) == 0xE0 ? 3:
			   (firstByte & 0xF8) == 0xF0 ? 4:
			   -1; // invalid UTF-8 character
	}
	
	metaffi_char8()=default;
	metaffi_char8(const char8_t* utf8c)
	{
		*this = utf8c;
	}
	
	metaffi_char8& operator=(const char8_t* utf8c)
	{
		int8_t required_bytes = num_of_bytes(utf8c);
		if(required_bytes == -1)
		{
			throw std::invalid_argument("Invalid UTF-8 character");
		}
		
		memcpy(this->c, utf8c, required_bytes);
		for(int8_t i=required_bytes ; i<4; i++)
		{
			this->c[i] = u8'\0';
		}
		
		return *this;
	}
	
	explicit operator metaffi_char16() const;
	explicit operator metaffi_char32() const;
#endif // __cplusplus
};

struct metaffi_char16
{
	char16_t c[2];

#ifdef __cplusplus
	static int8_t num_of_bytes(const char16_t* c)
	{
		auto firstShort = static_cast<uint16_t>(c[0]);
		return (firstShort >= 0xD800 && firstShort <= 0xDBFF) ? 4 : 2;
	}
	
	metaffi_char16()=default;
	explicit metaffi_char16(const char16_t* utf16c)
	{
		*this = utf16c;
	}
	
	metaffi_char16& operator=(const char16_t* utf16c)
	{
		int8_t required_bytes = num_of_bytes(utf16c);
		if(required_bytes == -1)
		{
			throw std::invalid_argument("Invalid UTF-16 character");
		}
		
		memcpy(this->c, utf16c, required_bytes);
		if(required_bytes == 2){
			this->c[1] = u'\0';
		}
		
		return *this;
	}
	
	[[nodiscard]] bool is_surrogate() const
	{
		return (this->c[0] >= 0xD800 && this->c[0] <= 0xDFFF);
	}
	
	explicit operator metaffi_char8() const;
	explicit operator metaffi_char32() const;
	
#endif // __cplusplus
};

struct metaffi_char32
{
	char32_t c;
	
#ifdef __cplusplus
	
	metaffi_char32():c(0){}
	explicit metaffi_char32(char32_t c) : c(c) {}
	
	explicit operator metaffi_char8() const;
	explicit operator metaffi_char16() const;
#endif // __cplusplus
};

typedef char8_t* metaffi_string8;
typedef char16_t* metaffi_string16;
typedef char32_t* metaffi_string32;

typedef uint64_t metaffi_size; // sizes of array and/or strings passed by this type
typedef void* metaffi_handle;
typedef metaffi_size metaffi_type;

typedef void* metaffi_callable;

typedef uint64_t* metaffi_types_ptr;
typedef uint64_t metaffi_type_t;

enum metaffi_types
{
	metaffi_float64_type = (metaffi_type)1ULL,
	metaffi_float32_type = (metaffi_type)2ULL,
	metaffi_int8_type = (metaffi_type)4ULL,
	metaffi_int16_type = (metaffi_type)8ULL,
	metaffi_int32_type = (metaffi_type)16ULL,
	metaffi_int64_type = (metaffi_type)32ULL,
	metaffi_uint8_type = (metaffi_type)64ULL,
	metaffi_uint16_type = (metaffi_type)128ULL,
	metaffi_uint32_type = (metaffi_type)256ULL,
	metaffi_uint64_type = (metaffi_type)512ULL,
	metaffi_bool_type = (metaffi_type)1024ULL,
	
	metaffi_char8_type = (metaffi_type)524288ULL,
	metaffi_char16_type = (metaffi_type)1048576ULL,
	metaffi_char32_type = (metaffi_type)2097152ULL,
	
	metaffi_string8_type = (metaffi_type)4096ULL,
	metaffi_string16_type = (metaffi_type)8192ULL,
	metaffi_string32_type = (metaffi_type)16384ULL,
	
	metaffi_handle_type = (metaffi_type)32768ULL,
	
	metaffi_array_type = (metaffi_type)65536ULL,
	
	metaffi_size_type = (metaffi_type)262144ULL,
	
	metaffi_any_type = (metaffi_type)4194304ULL,
	
	metaffi_null_type = (metaffi_type)8388608ULL,

	metaffi_callable_type = (metaffi_type)16777216ULL,

	metaffi_float64_array_type = (metaffi_type)(metaffi_float64_type | metaffi_array_type),
	metaffi_float32_array_type = (metaffi_type)(metaffi_float32_type | metaffi_array_type),
	metaffi_int8_array_type = (metaffi_type)(metaffi_int8_type | metaffi_array_type),
	metaffi_int16_array_type = (metaffi_type)(metaffi_int16_type | metaffi_array_type),
	metaffi_int32_array_type = (metaffi_type)(metaffi_int32_type | metaffi_array_type),
	metaffi_int64_array_type = (metaffi_type)(metaffi_int64_type | metaffi_array_type),
	metaffi_uint8_array_type = (metaffi_type)(metaffi_uint8_type | metaffi_array_type),
	metaffi_uint16_array_type = (metaffi_type)(metaffi_uint16_type | metaffi_array_type),
	metaffi_uint32_array_type = (metaffi_type)(metaffi_uint32_type | metaffi_array_type),
	metaffi_uint64_array_type = (metaffi_type)(metaffi_uint64_type | metaffi_array_type),
	metaffi_bool_array_type = (metaffi_type)(metaffi_bool_type | metaffi_array_type),
	metaffi_char8_array_type = (metaffi_type)(metaffi_char8_type | metaffi_array_type),
	metaffi_string8_array_type = (metaffi_type)(metaffi_string8_type | metaffi_array_type),
	metaffi_char16_array_type = (metaffi_type)(metaffi_char16_type | metaffi_array_type),
	metaffi_string16_array_type = (metaffi_type)(metaffi_string16_type | metaffi_array_type),
	metaffi_char32_array_type = (metaffi_type)(metaffi_char32_type | metaffi_array_type),
	metaffi_string32_array_type = (metaffi_type)(metaffi_string32_type | metaffi_array_type),
	
	metaffi_handle_array_type = (metaffi_type)(metaffi_handle_type | metaffi_array_type),
	metaffi_size_array_type = (metaffi_type)(metaffi_size_type | metaffi_array_type)
};

#define metaffi_type_to_str(t, str) \
    str = (t == metaffi_float64_type) ? "metaffi_float64" : \
          (t == metaffi_float32_type) ? "metaffi_float32" : \
          (t == metaffi_int8_type) ? "metaffi_int8" : \
          (t == metaffi_int16_type) ? "metaffi_int16" : \
          (t == metaffi_int32_type) ? "metaffi_int32" : \
          (t == metaffi_int64_type) ? "metaffi_int64" : \
          (t == metaffi_uint8_type) ? "metaffi_uint8" : \
          (t == metaffi_uint16_type) ? "metaffi_uint16" : \
          (t == metaffi_uint32_type) ? "metaffi_uint32" : \
          (t == metaffi_uint64_type) ? "metaffi_uint64" : \
          (t == metaffi_bool_type) ? "metaffi_bool" : \
          (t == metaffi_char8_type) ? "metaffi_char8" : \
          (t == metaffi_char16_type) ? "metaffi_char16" : \
          (t == metaffi_char32_type) ? "metaffi_char32" : \
          (t == metaffi_string8_type) ? "metaffi_string8" : \
          (t == metaffi_string16_type) ? "metaffi_string16" : \
          (t == metaffi_string32_type) ? "metaffi_string32" : \
          (t == metaffi_handle_type) ? "metaffi_handle" : \
          (t == metaffi_array_type) ? "metaffi_array" : \
          (t == metaffi_size_type) ? "metaffi_size" : \
          (t == metaffi_any_type) ? "metaffi_any" : \
          (t == metaffi_null_type) ? "metaffi_null" : \
          (t == metaffi_float64_array_type) ? "metaffi_float64_array" : \
          (t == metaffi_float32_array_type) ? "metaffi_float32_array" : \
          (t == metaffi_int8_array_type) ? "metaffi_int8_array" : \
          (t == metaffi_int16_array_type) ? "metaffi_int16_array" : \
          (t == metaffi_int32_array_type) ? "metaffi_int32_array" : \
          (t == metaffi_int64_array_type) ? "metaffi_int64_array" : \
          (t == metaffi_uint8_array_type) ? "metaffi_uint8_array" : \
          (t == metaffi_uint16_array_type) ? "metaffi_uint16_array" : \
          (t == metaffi_uint32_array_type) ? "metaffi_uint32_array" : \
          (t == metaffi_uint64_array_type) ? "metaffi_uint64_array" : \
          (t == metaffi_bool_array_type) ? "metaffi_bool_array" : \
          (t == metaffi_char8_array_type) ? "metaffi_char8_array" : \
          (t == metaffi_string8_array_type) ? "metaffi_string8_array" : \
          (t == metaffi_string16_array_type) ? "metaffi_string16_array" : \
          (t == metaffi_string32_array_type) ? "metaffi_string32_array" : \
          (t == metaffi_any_array_type) ? "metaffi_any_array" : \
          (t == metaffi_handle_array_type) ? "metaffi_handle_array" : \
          (t == metaffi_size_array_type) ? "metaffi_size_array" : \
		  (t == metaffi_callable_type) ? "metaffi_callable_type" : \
		  "Unknown type"


/**
 * @brief A MetaFFI type information
 */
struct metaffi_type_info
{
	metaffi_type type;
	char* alias;
	metaffi_bool is_free_alias;
	metaffi_int64 fixed_dimensions;

#define MIXED_OR_UNKNOWN_DIMENSIONS (-1)

#ifdef __cplusplus
	metaffi_type_info() : type(metaffi_null_type), alias(nullptr), is_free_alias(false), fixed_dimensions(MIXED_OR_UNKNOWN_DIMENSIONS) {}
	metaffi_type_info(metaffi_type type) : type(type), alias(nullptr), is_free_alias(false), fixed_dimensions(MIXED_OR_UNKNOWN_DIMENSIONS) {}
	metaffi_type_info(metaffi_type type, const char* alias, bool is_copy_alias = false, int64_t fixed_dimensions = MIXED_OR_UNKNOWN_DIMENSIONS) : type(type), fixed_dimensions(fixed_dimensions), is_free_alias(false)
	{
		if(is_copy_alias && alias)
		{
			size_t alias_length = std::strlen(alias);
			this->alias = new char[alias_length+1];
			memcpy(this->alias, alias, alias_length);
			this->alias[alias_length] = '\0';
			is_free_alias = true;
		}
		else
		{
			is_free_alias = false;
			this->alias = const_cast<char*>(alias);
		}
	}
	
	metaffi_type_info(metaffi_type_info&& other) noexcept: alias(nullptr), is_free_alias(false), fixed_dimensions(MIXED_OR_UNKNOWN_DIMENSIONS), type(metaffi_null_type){ *this = std::move(other); }
	metaffi_type_info(const metaffi_type_info& other):alias(nullptr), is_free_alias(false), fixed_dimensions(0), type(metaffi_null_type){ *this = other; }
	
	void set_copy_alias(const char* palias, int len = -1)
	{
		size_t alias_length = len > -1 ? len : std::strlen(palias);
		this->alias = new char[alias_length+1];
		memcpy(this->alias, palias, alias_length);
		this->alias[alias_length] = '\0';
		is_free_alias = true;
	}
	
	metaffi_type_info& operator=(const metaffi_type_info& other)
	{
		if(this != &other)
		{
			type = other.type;
			if(other.alias)
			{
				size_t alias_length = std::strlen(other.alias);
				alias = new char[alias_length+1];
				std::string_view(other.alias).copy(alias, alias_length);
				alias[alias_length] = '\0';
				is_free_alias = true;
			}
			
			fixed_dimensions = other.fixed_dimensions;
		}
		
		return *this;
	}
	
	metaffi_type_info& operator=(metaffi_type_info&& other) noexcept
	{
		if(this != &other)
		{
			type = other.type;
			alias = other.alias;
			is_free_alias = other.is_free_alias;
			fixed_dimensions = other.fixed_dimensions;
			other.alias = nullptr;
			other.is_free_alias = false;
		}
		
		return *this;
	}
	
	~metaffi_type_info()
	{
		if(is_free_alias)
		{
			delete[] alias;
		}
	}
#endif // __cplusplus
};


/**
 * @brief A handle to a foreign object. Allows passing objects between different runtimes
 * */
struct cdt_metaffi_handle
{
	metaffi_handle handle;
	uint64_t runtime_id;
	void (*release)(struct cdt_metaffi_handle*);

#ifdef __cplusplus
	cdt_metaffi_handle() : handle(nullptr), runtime_id(0), release(nullptr) {}
	cdt_metaffi_handle(metaffi_handle val, uint64_t runtime_id, void(*release)(cdt_metaffi_handle*)) : handle(val), runtime_id(runtime_id), release(release) {}
	bool operator==(const cdt_metaffi_handle& other) const { return handle == other.handle && runtime_id == other.runtime_id; }
#endif
};
#ifndef __cplusplus
typedef void(*release_fptr)(struct cdt_metaffi_handle*);
#endif

typedef void(*releaser_fptr_t)(struct cdt_metaffi_handle*);

/**
 * @brief A MetaFFI callable object. Allows passing callable objects between different runtimes
 * */
struct cdt_metaffi_callable
{
	metaffi_callable val;
	metaffi_type* parameters_types;
	metaffi_int8 params_types_length;
	metaffi_type* retval_types;
	metaffi_int8 retval_types_length;

#ifdef __cplusplus
	cdt_metaffi_callable() : val(nullptr), parameters_types(nullptr), params_types_length(0), retval_types(nullptr), retval_types_length(0) {}
	cdt_metaffi_callable(metaffi_callable val, metaffi_type* parameters_types, metaffi_int8 params_types_length, metaffi_type* retval_types, metaffi_int8 retval_types_length) : val(val), parameters_types(parameters_types), params_types_length(params_types_length), retval_types(retval_types), retval_types_length(retval_types_length) {}
	cdt_metaffi_callable(const cdt_metaffi_callable& other) : cdt_metaffi_callable(){ *this = other; }
	cdt_metaffi_callable(cdt_metaffi_callable&& other) noexcept : val(nullptr), parameters_types(nullptr), params_types_length(0),
																	retval_types(nullptr), retval_types_length(0) { *this = std::move(other); }
	cdt_metaffi_callable(metaffi_callable val, const std::vector<metaffi_type>& parameters_types, const std::vector<metaffi_type>& retval_types) : val(val)
	{
		params_types_length = static_cast<metaffi_int8>(parameters_types.size());
		retval_types_length = static_cast<metaffi_int8>(retval_types.size());
		this->parameters_types = new metaffi_type[params_types_length];
		this->retval_types = new metaffi_type[retval_types_length];
		memcpy(this->parameters_types, parameters_types.data(), sizeof(metaffi_type) * params_types_length);
		memcpy(this->retval_types, retval_types.data(), sizeof(metaffi_type) * retval_types_length);
	}
	
	~cdt_metaffi_callable()
	{
		free();
	}
	
	cdt_metaffi_callable& operator=(const cdt_metaffi_callable& other) noexcept
	{
		// copy other to this
		if(this != &other)
		{
			val = other.val;
			
			delete[] parameters_types;
			delete[] retval_types;
			
			retval_types_length = other.retval_types_length;
			params_types_length = other.params_types_length;
			parameters_types = new metaffi_type[params_types_length];
			retval_types = new metaffi_type[retval_types_length];
			memcpy(parameters_types, other.parameters_types, sizeof(metaffi_type) * params_types_length);
			memcpy(retval_types, other.retval_types, sizeof(metaffi_type) * retval_types_length);
		}
		
		return *this;
	}
	
	cdt_metaffi_callable& operator=(cdt_metaffi_callable&& other) noexcept
	{
		if(this != &other)
		{
			delete[] parameters_types;
			delete[] retval_types;
			
			val = other.val;
			parameters_types = other.parameters_types;
			params_types_length = other.params_types_length;
			retval_types = other.retval_types;
			retval_types_length = other.retval_types_length;
			other.parameters_types = nullptr;
			other.retval_types = nullptr;
			other.params_types_length = 0;
			other.retval_types_length = 0;
			other.val = nullptr;
		}
		
		return *this;
	}
	
	bool operator==(const cdt_metaffi_callable& other) const
	{
		if(val != other.val || params_types_length != other.params_types_length || retval_types_length != other.retval_types_length)
		{
			return false;
		}
		
		for(int i = 0; i < params_types_length; i++)
		{
			if(parameters_types[i] != other.parameters_types[i])
			{
				return false;
			}
		}
		
		for(int i = 0; i < retval_types_length; i++)
		{
			if(retval_types[i] != other.retval_types[i])
			{
				return false;
			}
		}
		
		return true;
	}
	
	void free()
	{
		delete[] parameters_types;
		delete[] retval_types;
		
		parameters_types = nullptr;
		retval_types = nullptr;
	}
#endif // __cplusplus
};

#ifdef __cplusplus
using metaffi_variant = std::variant<
		metaffi_float32,
		metaffi_float64,
		metaffi_int8,
		metaffi_uint8,
		metaffi_int16,
		metaffi_uint16,
		metaffi_int32,
		metaffi_uint32,
		metaffi_int64,
		metaffi_uint64,
		metaffi_char8,
		metaffi_char16,
		metaffi_char32,
		metaffi_string8,
		metaffi_string16,
		metaffi_string32,
		cdt_metaffi_handle,
		cdt_metaffi_callable
>;
#endif // __cplusplus

#endif // METAFFI_PRIMITIVES_H