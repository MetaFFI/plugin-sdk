#include "xllr_capi_loader.h"
#include <iostream>

bool xllr_capi_loaded = false;
struct xllr_capi_loader
{
	xllr_capi_loader()
	{
		if(!xllr_capi_loaded)
		{
			const char* loader_err = load_xllr();
			if(loader_err)
			{
				std::cerr << "FATAL ERROR! Failed to load XLLR C-API. Error: " << loader_err << std::endl;
				exit(1);
			}
			
			xllr_capi_loaded = true;
		}
	}
};
static xllr_capi_loader l; // load statically the XLLR C-API
