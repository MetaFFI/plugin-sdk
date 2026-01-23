#include "env_utils.h"

#include <cstdlib>

std::string get_env_var(const char* name)
{
#if defined(_MSC_VER)
	char* value = nullptr;
	size_t value_size = 0;
	if(_dupenv_s(&value, &value_size, name) != 0 || value == nullptr)
	{
		return std::string();
	}

	std::string result(value);
	free(value);
	return result;
#else
	const char* value = std::getenv(name);
	return value ? std::string(value) : std::string();
#endif
}
