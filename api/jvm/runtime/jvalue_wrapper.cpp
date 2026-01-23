#include "jvalue_wrapper.h"

jvalue_wrapper::jvalue_wrapper(jvalue v, char t): value(v), type(t){}

jvalue jvalue_wrapper::get_value() const
{
	return value;
}

char jvalue_wrapper::get_type() const
{
	return type;
}

jvalue_wrapper::operator jvalue() const
{
	return value;
}


