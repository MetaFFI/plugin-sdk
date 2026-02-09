# Go API is a Go module; built by Go tooling (go build), not by CMake.
# Distribution: consumers use go get github.com/MetaFFI/sdk/api/go
# XLLR/CDT runtime helpers live in the metaffi subpackage (absorbed from go-runtime).
add_custom_target(api_go)
set(api_go api_go PARENT_SCOPE)

# Go API unit tests (go test against xllr.test plugin)
add_go_test(api_go_unittest WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/unittest)
