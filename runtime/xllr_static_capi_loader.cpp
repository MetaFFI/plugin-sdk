#include "xllr_capi_loader.h"
#include <utils/logger.hpp>

static auto LOG = metaffi::get_logger("sdk.runtime");

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
				METAFFI_CRITICAL(LOG, "Failed to load XLLR C-API. Error: {}", loader_err);
				exit(1);
			}
			
			xllr_capi_loaded = true;
		}
	}
};
static xllr_capi_loader l; // load statically the XLLR C-API
