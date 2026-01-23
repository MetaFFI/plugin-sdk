set(GO_COMPILER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

add_custom_target(compiler_go_tests)

add_go_test(compiler_go_tests WORKING_DIRECTORY ${GO_COMPILER_SRC_DIR})

set(compiler_go_tests compiler_go_tests PARENT_SCOPE)
