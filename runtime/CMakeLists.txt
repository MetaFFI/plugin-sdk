include("${CMAKE_CURRENT_LIST_DIR}/run_sdk_tests.cmake")

add_custom_target(sdk_runtime
	DEPENDS cdts_test xllr_capi_test
)

set(sdk_runtime sdk_runtime PARENT_SCOPE)
