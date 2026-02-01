#include "env_utils.h"

#include <utils/safe_func.h>

std::string get_env_var(const char* name)
{
	char* value = metaffi_getenv_alloc(name);
	if(!value)
	{
		return std::string();
	}

	std::string result(value);
	metaffi_free_env(value);
	return result;
}
