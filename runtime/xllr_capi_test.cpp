#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "xllr_capi_loader.h"
#include "cdt.h"

TEST_SUITE("XLLR CAPI Loader")
{
	TEST_CASE("Alloc CDTS Buffer")
	{
		cdts* buf = xllr_alloc_cdts_buffer(1, 1);
		REQUIRE((buf != nullptr));
		REQUIRE((buf[0].length == 1));
		REQUIRE((buf[1].length == 1));

		REQUIRE((buf[0].arr != nullptr));
		REQUIRE((buf[1].arr != nullptr));
		
		xllr_free_cdts_buffer(buf);
		buf = nullptr;
	}
}