#pragma once

#include <jni.h>

class jvalue_wrapper
{
private:
	jvalue value;
	char type;
	
public:
	jvalue_wrapper(jvalue v, char t);
	
	[[nodiscard]] jvalue get_value() const;
	[[nodiscard]] char get_type() const;
	
	explicit operator jvalue() const;
};
