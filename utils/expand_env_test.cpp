#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "expand_env.h"


TEST_CASE( "Expand Environment Variable", "[sdk]" )
{
	using namespace metaffi::utils;
	
	REQUIRE(std::getenv("METAFFI_HOME") != nullptr);
	
	std::string original = std::string("Text ")+std::getenv("METAFFI_HOME")+std::string(" Text");
	
	SECTION("Windows-Style (%X%)")
	{
		REQUIRE(expand_env("Text %METAFFI_HOME% Text") == original);
	}

	SECTION("*nix Style ($X)")
	{
		REQUIRE(expand_env("Text $METAFFI_HOME Text") == original);
	}
	
	SECTION("PowerShell Style ($ENV:X)")
	{
		REQUIRE(expand_env("Text $Env:METAFFI_HOME Text") == original);
	}
	
	SECTION("*nix Style 2 (${X})")
	{
		REQUIRE(expand_env("Text ${METAFFI_HOME} Text") == original);
	}
	
	char cwd[1024] = {0};

#ifdef _WIN32
	GetCurrentDirectory(MAX_PATH,cwd);
#else
	getcwd(cwd, 1024);
#endif
	
	SECTION("Current Dir CD")
	{
		REQUIRE(expand_env("%CD%") == cwd);
	}
	
	SECTION("Current Dir PWD")
	{
		REQUIRE(expand_env("$PWD") == cwd);
	}
}