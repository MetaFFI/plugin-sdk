# add all C/CPP files to sdk_include_dir, sdk_src_files, sdk_header_files
# add them to the parent scope
collect_c_cpp_files("${CMAKE_CURRENT_LIST_DIR};${CMAKE_CURRENT_LIST_DIR}/compiler;${CMAKE_CURRENT_LIST_DIR}/runtime;${CMAKE_CURRENT_LIST_DIR}/utils" sdk)

# sdk_src and sdk_include to the parent scope
set(sdk_src ${sdk_src} PARENT_SCOPE)
set(sdk_include ${sdk_include} PARENT_SCOPE)
set(sdk_include_dir ${sdk_include_dir} PARENT_SCOPE)
