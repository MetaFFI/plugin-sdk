#include "argument_definition.h"

#include <algorithm>
#include <sstream>

argument_definition::argument_definition(const metaffi_type_info& type_alias)
{
	type = type_alias;
	alias = type.alias != nullptr ? type.alias : "";
}

std::string argument_definition::to_jni_signature_type() const
{
	std::stringstream ss;
	
	if(!alias.empty())
	{
		std::string tmp(alias);
		std::replace(tmp.begin(), tmp.end(), '.', '/');
		
		// if alias is array.
		size_t pos = std::string::npos;
		while((pos = tmp.find("[]")) != std::string::npos)
		{
			tmp.replace(pos, 2, "");
			ss << "[";
		}
		
		ss << "L"  << tmp << ";";
		
		return ss.str();
	}
	
	if(type.type & metaffi_array_type){
		ss << std::string(type.fixed_dimensions, '[');
	}
	
	metaffi_type tmp = type.type & (~metaffi_array_type);
	
	if (tmp == metaffi_null_type) ss << "V";
	else if (tmp == metaffi_bool_type) ss << "Z";
	else if (tmp == metaffi_int8_type || tmp == metaffi_uint8_type) ss << "B";
	else if (tmp == metaffi_char8_type) ss << "C";
	else if (tmp == metaffi_int16_type || tmp == metaffi_uint16_type) ss << "S";
	else if (tmp == metaffi_int32_type || tmp == metaffi_uint32_type) ss << "I";
	else if (tmp == metaffi_int64_type || tmp == metaffi_uint64_type) ss << "J";
	else if (tmp == metaffi_float32_type) ss << "F";
	else if (tmp == metaffi_float64_type) ss << "D";
	else if (tmp == metaffi_string8_type) ss << "Ljava/lang/String;";
	else if (tmp == metaffi_any_type) ss << "Ljava/lang/Object;";
	else if (tmp == metaffi_handle_type) ss << "Ljava/lang/Object;";
	else if (tmp == metaffi_callable_type) ss << "Lmetaffi/api/accessor/Caller;";

	return ss.str();
}

