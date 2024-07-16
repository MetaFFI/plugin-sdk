#ifndef XCALL_H
#define XCALL_H

#pragma once
#include "cdt.h"
#include "xllr_capi_loader.h"


#ifndef __cplusplus
struct xcall
{
	void* pxcall_and_context[2];
};
#else

// make sure you include utils/scope_guard.hpp
// if_fail_code need to assume "char* err" is defined
#define xcall_scope_guard(plugin, name, if_fail_code) \
	metaffi::utils::scope_guard sg_##name([&name]() \
	{ \
		if(name && name->is_valid()) \
		{ \
            char* err = nullptr;         \
			xllr_free_xcall(plugin, name, &err);   \
            if(err)                           \
            {                          \
                if_fail_code;  \
			}                                  \
                                              \
            name = nullptr;     \
		} \
	});

struct xcall
{
	void* pxcall_and_context[2];
	
	xcall()
	{
		pxcall_and_context[0] = nullptr;
		pxcall_and_context[1] = nullptr;
	}
	
	explicit xcall(void* pxcall_and_context[2])
	{
		this->pxcall_and_context[0] = pxcall_and_context[0];
		this->pxcall_and_context[1] = pxcall_and_context[1];
	}
	
	explicit xcall(void* pxcall, void* context)
	{
		if(!pxcall)
		{
			throw std::invalid_argument("pxcall must not be null");
		}
		
		pxcall_and_context[0] = pxcall;
		pxcall_and_context[1] = context;
	}
	
	// copy constructor
	xcall(const xcall& other)
	{
		*this = other;
	}
	
	// move constructor
	xcall(xcall&& other) noexcept
	{
		*this = std::move(other);
	}
	
	[[nodiscard]] bool is_valid() const
	{
		return pxcall_and_context[0] != nullptr;
	}
	
	// assignment operator
	xcall& operator=(const xcall& other)
	{
		if(this != &other)
		{
			pxcall_and_context[0] = other.pxcall_and_context[0];
			pxcall_and_context[1] = other.pxcall_and_context[1];
		}
		return *this;
	}
	
	// move operator
	xcall& operator=(xcall&& other) noexcept
	{
		if(this != &other)
		{
			pxcall_and_context[0] = other.pxcall_and_context[0];
			pxcall_and_context[1] = other.pxcall_and_context[1];
			
			other.pxcall_and_context[0] = nullptr;
			other.pxcall_and_context[1] = nullptr;
		}
		return *this;
	}
	
	explicit operator bool () const
	{
		return is_valid();
	}
	
	// use ONLY if the function foreign function has no parameters and no return values
	void operator()(char** err)
	{
		if(pxcall_and_context[0] == nullptr){
			throw std::invalid_argument("pxcall_and_context[0] must not be null");
		}

		((void (*)(void*, char**))pxcall_and_context[0])(pxcall_and_context[1], err);
	}
	
	void operator()()
	{
		if(pxcall_and_context[0] == nullptr){
			throw std::invalid_argument("pxcall_and_context[0] must not be null");
		}

		char* err = nullptr;
		((void (*)(void*, char**))pxcall_and_context[0])(pxcall_and_context[1], &err);
		
		if(err)
		{
			std::string err_str(err);
			free(err);
			err = nullptr;
			
			throw std::runtime_error(err);
		}
	}
	
	void operator()(struct cdts* pcdts, char** err)
	{
		if(pxcall_and_context[0] == nullptr){
			throw std::invalid_argument("pxcall_and_context[0] must not be null");
		}

		((void (*)(void*, struct cdts*, char**))pxcall_and_context[0])(pxcall_and_context[1], pcdts, err);
	}
	
	void operator()(struct cdts* pcdts)
	{
		if(pxcall_and_context[0] == nullptr){
			throw std::invalid_argument("pxcall_and_context[0] must not be null");
		}

		char* err = nullptr;
		((void (*)(void*, struct cdts*, char**))pxcall_and_context[0])(pxcall_and_context[1], pcdts, &err);
		
		if(err)
		{
			std::string err_str(err);
			free(err);
			err = nullptr;
			
			throw std::runtime_error(err);
		}
	}
};
#endif

#ifndef __cplusplus
void call_xcall_no_params_no_retvals(struct xcall* pxcall, char** err)
{
	((void (*)(void*, char**))pxcall->pxcall_and_context[0])(pxcall->pxcall_and_context[1], err);
}
void call_xcall_with_params_and_or_retvals(struct xcall* pxcall, struct cdts* pcdts, char** err)
{
	((void (*)(void*, struct cdts*, char**))pxcall->pxcall_and_context[0])(pxcall->pxcall_and_context[1], pcdts, err);
}
#endif


#endif // XCALL_H