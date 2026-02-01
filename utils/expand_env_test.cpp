#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>
#include "expand_env.h"
#include <utils/safe_func.h>
#include <filesystem>
using namespace metaffi::utils;

std::string original;

struct setup
{
	setup()
	{
		char* metaffi_home = metaffi_getenv_alloc("METAFFI_HOME");
		if(metaffi_home)
		{
			original = std::string("Text ") + metaffi_home + std::string(" Text");
			metaffi_free_env(metaffi_home);
		}
	}
};
static setup s;

TEST_SUITE( "Expand Environment Variable")
{
	TEST_CASE("Windows-Style (%X%)")
	{
		REQUIRE(expand_env("Text %METAFFI_HOME% Text") == original);
	}
	
	TEST_CASE("*nix Style ($X)")
	{
		REQUIRE(expand_env("Text $METAFFI_HOME Text") == original);
	}
	
	TEST_CASE("PowerShell Style ($ENV:X)")
	{
		REQUIRE(expand_env("Text $Env:METAFFI_HOME Text") == original);
	}
	
	TEST_CASE("*nix Style 2 (${X})")
	{
		REQUIRE(expand_env("Text ${METAFFI_HOME} Text") == original);
	}
	
	TEST_CASE("Current Dir CD")
	{
		std::filesystem::path currentDir = std::filesystem::current_path();
		REQUIRE(expand_env("%CD%") == currentDir.string());
	}
	
	TEST_CASE("Current Dir PWD")
	{
		std::filesystem::path currentDir = std::filesystem::current_path();
		REQUIRE(expand_env("$PWD") == currentDir.string());
	}
}
