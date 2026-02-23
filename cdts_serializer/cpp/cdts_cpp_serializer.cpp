#include "cdts_cpp_serializer.h"
#include <cstring>
#include <runtime/xllr_capi_loader.h>
#include <cdts_serializer/cpp/runtime_id.h>

namespace metaffi::utils
{

// ===== Constructor =====

cdts_cpp_serializer::cdts_cpp_serializer(cdts& pcdts)
	: data(pcdts), current_index(0)
{
}

// ===== Helper Methods =====

void cdts_cpp_serializer::check_bounds(metaffi_size index) const
{
	if (index >= data.length)
	{
		std::stringstream ss;
		ss << "Index out of bounds: " << index << " >= " << data.length;
		throw std::out_of_range(ss.str());
	}
}

// ===== SERIALIZATION (C++ → CDT) =====

// Primitives (standard C++ types)
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int8_t val)
{
	check_bounds(current_index);
	data[current_index] = val;  // Use CDT's assignment operator
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int16_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int32_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int64_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint8_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint16_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint32_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint64_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(float val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(double val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(bool val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

#if defined(__linux__)
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(long long val)
{
	return (*this) << static_cast<int64_t>(val);
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(unsigned long long val)
{
	return (*this) << static_cast<uint64_t>(val);
}
#endif

// Strings
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::string& val)
{
	check_bounds(current_index);
	data[current_index].set_string((const char8_t*)val.c_str(), true);  // is_copy=true
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::u16string& val)
{
	check_bounds(current_index);
	data[current_index].set_string(val.c_str(), true);  // is_copy=true
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::u32string& val)
{
	check_bounds(current_index);
	data[current_index].set_string(val.c_str(), true);  // is_copy=true
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const char* val)
{
	check_bounds(current_index);
	data[current_index].set_string((const char8_t*)val, true);  // is_copy=true
	current_index++;
	return *this;
}

// Characters
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_char8& val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_char16& val)
{
	check_bounds(current_index);
	data[current_index].type = metaffi_char16_type;
	data[current_index].cdt_val.char16_val = val;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_char32& val)
{
	check_bounds(current_index);
	data[current_index].type = metaffi_char32_type;
	data[current_index].cdt_val.char32_val = val;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

// Handles
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(metaffi_handle val)
{
	check_bounds(current_index);

	void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
	if(!handle_mem)
	{
		throw std::runtime_error("Failed to allocate memory for cdt_metaffi_handle");
	}

	cdt_metaffi_handle* handle = new (handle_mem) cdt_metaffi_handle();
	handle->handle = val;
	handle->runtime_id = CPP_RUNTIME_ID;
	handle->release = nullptr;

	data[current_index].type = metaffi_handle_type;
	data[current_index].cdt_val.handle_val = handle;
	data[current_index].free_required = true;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const cdt_metaffi_handle& handle)
{
	check_bounds(current_index);
	data[current_index].set_handle(&handle);
	current_index++;
	return *this;
}

// Callables
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const cdt_metaffi_callable& callable)
{
	check_bounds(current_index);
	
	// Create a copy of the callable with xllr-allocated arrays
	// The input callable might have stack-allocated arrays, so we need to copy them to xllr memory
	void* callable_mem = xllr_alloc_memory(sizeof(cdt_metaffi_callable));
	if(!callable_mem)
	{
		throw std::runtime_error("Failed to allocate callable memory");
	}
	cdt_metaffi_callable* callable_copy = new (callable_mem) cdt_metaffi_callable();
	callable_copy->val = callable.val;
	callable_copy->params_types_length = callable.params_types_length;
	callable_copy->retval_types_length = callable.retval_types_length;
	
	// Allocate and copy parameter types
	if (callable.params_types_length > 0) {
		void* params_mem = xllr_alloc_memory(sizeof(metaffi_type) * callable.params_types_length);
		if(!params_mem)
		{
			xllr_free_memory(callable_mem);
			throw std::runtime_error("Failed to allocate callable parameters memory");
		}
		callable_copy->parameters_types = static_cast<metaffi_type*>(params_mem);
		memcpy(callable_copy->parameters_types, callable.parameters_types, sizeof(metaffi_type) * callable.params_types_length);
	} else {
		callable_copy->parameters_types = nullptr;
	}
	
	// Allocate and copy return value types
	if (callable.retval_types_length > 0) {
		void* ret_mem = xllr_alloc_memory(sizeof(metaffi_type) * callable.retval_types_length);
		if(!ret_mem)
		{
			if(callable_copy->parameters_types)
			{
				xllr_free_memory(callable_copy->parameters_types);
			}
			xllr_free_memory(callable_mem);
			throw std::runtime_error("Failed to allocate callable return types memory");
		}
		callable_copy->retval_types = static_cast<metaffi_type*>(ret_mem);
		memcpy(callable_copy->retval_types, callable.retval_types, sizeof(metaffi_type) * callable.retval_types_length);
	} else {
		callable_copy->retval_types = nullptr;
	}
	
	data[current_index].type = metaffi_callable_type;
	data[current_index].cdt_val.callable_val = callable_copy;
	data[current_index].free_required = true;  // CDT owns the callable copy
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(std::nullptr_t)
{
	check_bounds(current_index);
	data[current_index].type = metaffi_null_type;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_variant& val)
{
	check_bounds(current_index);

	std::visit([this](auto&& v)
	{
		using V = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<V, metaffi_float32> || std::is_same_v<V, metaffi_float64> ||
		              std::is_same_v<V, metaffi_int8> || std::is_same_v<V, metaffi_uint8> ||
		              std::is_same_v<V, metaffi_int16> || std::is_same_v<V, metaffi_uint16> ||
		              std::is_same_v<V, metaffi_int32> || std::is_same_v<V, metaffi_uint32> ||
		              std::is_same_v<V, metaffi_int64> || std::is_same_v<V, metaffi_uint64> ||
		              std::is_same_v<V, metaffi_char8> || std::is_same_v<V, metaffi_char16> ||
		              std::is_same_v<V, metaffi_char32> || std::is_same_v<V, cdt_metaffi_handle> ||
		              std::is_same_v<V, cdt_metaffi_callable>)
		{
			*this << v;
		}
		else if constexpr (std::is_same_v<V, metaffi_string8>)
		{
			if(v == nullptr)
			{
				this->null();
			}
			else
			{
				*this << std::string(reinterpret_cast<const char*>(v));
			}
		}
		else if constexpr (std::is_same_v<V, metaffi_string16>)
		{
			if(v == nullptr)
			{
				this->null();
			}
			else
			{
				*this << std::u16string(reinterpret_cast<const char16_t*>(v));
			}
		}
		else if constexpr (std::is_same_v<V, metaffi_string32>)
		{
			if(v == nullptr)
			{
				this->null();
			}
			else
			{
				*this << std::u32string(reinterpret_cast<const char32_t*>(v));
			}
		}
		else
		{
			this->null();
		}
	}, val);

	return *this;
}

// Null
cdts_cpp_serializer& cdts_cpp_serializer::null()
{
	check_bounds(current_index);
	data[current_index].type = metaffi_null_type;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

// ===== DESERIALIZATION (CDT → C++) =====

// Primitives (standard C++ types)
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int8_t& val)
{
	validate_type_at<int8_t>(current_index);
	val = static_cast<int8_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int16_t& val)
{
	validate_type_at<int16_t>(current_index);
	val = static_cast<int16_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int32_t& val)
{
	validate_type_at<int32_t>(current_index);
	val = static_cast<int32_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int64_t& val)
{
	validate_type_at<int64_t>(current_index);
	val = static_cast<int64_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint8_t& val)
{
	validate_type_at<uint8_t>(current_index);
	val = static_cast<uint8_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint16_t& val)
{
	validate_type_at<uint16_t>(current_index);
	val = static_cast<uint16_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint32_t& val)
{
	validate_type_at<uint32_t>(current_index);
	val = static_cast<uint32_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint64_t& val)
{
	validate_type_at<uint64_t>(current_index);
	val = static_cast<uint64_t>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(float& val)
{
	validate_type_at<float>(current_index);
	val = static_cast<float>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(double& val)
{
	validate_type_at<double>(current_index);
	val = static_cast<double>(data[current_index]);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(bool& val)
{
	validate_type_at<bool>(current_index);
	val = static_cast<bool>(data[current_index]);
	current_index++;
	return *this;
}

#if defined(__linux__)
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(long long& val)
{
	int64_t tmp = 0;
	(*this) >> tmp;
	val = static_cast<long long>(tmp);
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(unsigned long long& val)
{
	uint64_t tmp = 0;
	(*this) >> tmp;
	val = static_cast<unsigned long long>(tmp);
	return *this;
}
#endif

// Strings
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(std::string& val)
{
	validate_type_at<std::string>(current_index);
	val = std::string((const char*)data[current_index].cdt_val.string8_val);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(std::u16string& val)
{
	validate_type_at<std::u16string>(current_index);
	val = std::u16string(data[current_index].cdt_val.string16_val);
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(std::u32string& val)
{
	validate_type_at<std::u32string>(current_index);
	val = std::u32string(data[current_index].cdt_val.string32_val);
	current_index++;
	return *this;
}

// Characters
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_char8& val)
{
	validate_type_at<metaffi_char8>(current_index);
	val = data[current_index].cdt_val.char8_val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_char16& val)
{
	validate_type_at<metaffi_char16>(current_index);
	val = data[current_index].cdt_val.char16_val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_char32& val)
{
	validate_type_at<metaffi_char32>(current_index);
	val = data[current_index].cdt_val.char32_val;
	current_index++;
	return *this;
}

// Handles
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_handle& handle)
{
	validate_type_at<cdt_metaffi_handle>(current_index);
	handle = *data[current_index].cdt_val.handle_val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_handle*& handle)
{
	check_bounds(current_index);
	if(data[current_index].type != metaffi_handle_type)
	{
		std::stringstream ss;
		ss << "(cdts_cpp_serializer) Type mismatch at index " << current_index << ": expected handle, got type "
		   << data[current_index].type;
		throw std::runtime_error(ss.str());
	}
	handle = data[current_index].cdt_val.handle_val;
	data[current_index].cdt_val.handle_val = nullptr;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_handle& val)
{
	check_bounds(current_index);
	if(data[current_index].type != metaffi_handle_type)
	{
		std::stringstream ss;
		ss << "(cdts_cpp_serializer) Type mismatch at index " << current_index << ": expected handle, got type "
		   << data[current_index].type;
		throw std::runtime_error(ss.str());
	}

	cdt_metaffi_handle* handle = data[current_index].cdt_val.handle_val;
	if(handle == nullptr || handle->handle == nullptr)
	{
		val = nullptr;
		current_index++;
		return *this;
	}

	if(handle->runtime_id != CPP_RUNTIME_ID)
	{
		std::stringstream ss;
		ss << "Handle runtime_id mismatch at index " << current_index << ": expected "
		   << CPP_RUNTIME_ID << ", got " << handle->runtime_id;
		throw std::runtime_error(ss.str());
	}

	val = handle->handle;
	current_index++;
	return *this;
}

// Callables
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_callable& callable)
{
	validate_type_at<cdt_metaffi_callable>(current_index);
	// Shallow copy - just copy pointers
	// User must not free the arrays (they're owned by the CDT)
	callable = *data[current_index].cdt_val.callable_val;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_callable*& callable)
{
	check_bounds(current_index);
	if(data[current_index].type != metaffi_callable_type)
	{
		std::stringstream ss;
		ss << "(cdts_cpp_serializer) Type mismatch at index " << current_index << ": expected callable, got type "
		   << data[current_index].type;
		throw std::runtime_error(ss.str());
	}
	callable = data[current_index].cdt_val.callable_val;
	data[current_index].cdt_val.callable_val = nullptr;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_variant& val)
{
	val = extract_any();
	return *this;
}

// ===== ANY TYPE SUPPORT =====

cdts_cpp_serializer::cdts_any_variant cdts_cpp_serializer::extract_any()
{
	check_bounds(current_index);
	metaffi_type type = data[current_index].type;

	// Check if null
	if (type == metaffi_null_type)
	{
		current_index++;
		return cdt_metaffi_handle{};
	}

	// Check if array (NOTE: arrays not fully supported in variant yet)
	if (type & metaffi_array_type)
	{
		throw std::runtime_error("Array extraction via extract_any() not yet implemented - use operator>> with vector type");
	}

	// Extract based on type
	switch (type)
	{
		case metaffi_int8_type:
		{
			metaffi_int8 val = data[current_index].cdt_val.int8_val;
			current_index++;
			return val;
		}
		case metaffi_int16_type:
		{
			metaffi_int16 val = data[current_index].cdt_val.int16_val;
			current_index++;
			return val;
		}
		case metaffi_int32_type:
		{
			metaffi_int32 val = data[current_index].cdt_val.int32_val;
			current_index++;
			return val;
		}
		case metaffi_int64_type:
		{
			metaffi_int64 val = data[current_index].cdt_val.int64_val;
			current_index++;
			return val;
		}
		case metaffi_uint8_type:
		{
			metaffi_uint8 val = data[current_index].cdt_val.uint8_val;
			current_index++;
			return val;
		}
		case metaffi_uint16_type:
		{
			metaffi_uint16 val = data[current_index].cdt_val.uint16_val;
			current_index++;
			return val;
		}
		case metaffi_uint32_type:
		{
			metaffi_uint32 val = data[current_index].cdt_val.uint32_val;
			current_index++;
			return val;
		}
		case metaffi_uint64_type:
		{
			metaffi_uint64 val = data[current_index].cdt_val.uint64_val;
			current_index++;
			return val;
		}
		case metaffi_float32_type:
		{
			metaffi_float32 val = data[current_index].cdt_val.float32_val;
			current_index++;
			return val;
		}
		case metaffi_float64_type:
		{
			metaffi_float64 val = data[current_index].cdt_val.float64_val;
			current_index++;
			return val;
		}
		case metaffi_bool_type:
		{
			metaffi_uint8 val = data[current_index].cdt_val.bool_val ? 1 : 0;
			current_index++;
			return val;
		}
		case metaffi_char8_type:
		{
			metaffi_char8 val = data[current_index].cdt_val.char8_val;
			current_index++;
			return val;
		}
		case metaffi_char16_type:
		{
			metaffi_char16 val = data[current_index].cdt_val.char16_val;
			current_index++;
			return val;
		}
		case metaffi_char32_type:
		{
			metaffi_char32 val = data[current_index].cdt_val.char32_val;
			current_index++;
			return val;
		}
		case metaffi_string8_type:
		{
			cdt& source = data[current_index];
			metaffi_string8 val = source.cdt_val.string8_val;
			source.free_required = false;
			current_index++;
			return val;
		}
		case metaffi_string16_type:
		{
			cdt& source = data[current_index];
			metaffi_string16 val = source.cdt_val.string16_val;
			source.free_required = false;
			current_index++;
			return val;
		}
		case metaffi_string32_type:
		{
			cdt& source = data[current_index];
			metaffi_string32 val = source.cdt_val.string32_val;
			source.free_required = false;
			current_index++;
			return val;
		}
		case metaffi_handle_type:
		{
			cdt_metaffi_handle val;
			*this >> val;
			return val;
		}
		case metaffi_callable_type:
		{
			cdt_metaffi_callable val;
			*this >> val;
			return val;
		}
		default:
		{
			std::stringstream ss;
			ss << "Unknown type at index " << current_index << ": " << type;
			throw std::runtime_error(ss.str());
		}
	}
}

metaffi_type cdts_cpp_serializer::peek_type() const
{
	check_bounds(current_index);
	return data[current_index].type;
}

bool cdts_cpp_serializer::is_null() const
{
	check_bounds(current_index);
	return data[current_index].type == metaffi_null_type;
}

// ===== UTILITY METHODS =====

void cdts_cpp_serializer::reset()
{
	current_index = 0;
}

metaffi_size cdts_cpp_serializer::get_index() const
{
	return current_index;
}

void cdts_cpp_serializer::set_index(metaffi_size index)
{
	current_index = index;
}

metaffi_size cdts_cpp_serializer::size() const
{
	return data.length;
}

bool cdts_cpp_serializer::has_more() const
{
	return current_index < data.length;
}

} // namespace metaffi::utils
