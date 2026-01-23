# load conan packages
find_or_install_package(Boost COMPONENTS filesystem)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# get all java files in CMAKE_CURRENT_LIST_DIR (recursive for package subdirs)
file(GLOB_RECURSE JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/*.java)

# build metaffi.api.accessor native library
file(GLOB API_ACCESSOR_SRC ${CMAKE_CURRENT_LIST_DIR}/accessor/*.cpp)
file(GLOB API_RUNTIME_SRC ${CMAKE_CURRENT_LIST_DIR}/runtime/*.cpp)
set(API_JVM_SRC
	${metaffi_sdk_root}/runtime_manager/jdk/jvm.cpp
	${metaffi_sdk_root}/runtime_manager/jdk/jni_api_wrapper.cpp
)

c_cpp_shared_lib(metaffi.api.accessor
		"${API_ACCESSOR_SRC};${API_RUNTIME_SRC};${API_JVM_SRC};${sdk_src}"
		"${sdk_include_dir};${CMAKE_CURRENT_LIST_DIR};${CMAKE_CURRENT_LIST_DIR}/accessor;${CMAKE_CURRENT_LIST_DIR}/runtime;${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS};"
		"Boost::filesystem;${JNI_LIBRARIES}"
		"sdk/api/jvm")

set(metaffi.api.accessor metaffi.api.accessor PARENT_SCOPE)

# * ---- build metaffi.api.jar from *.java ----
build_jar(metaffi.api
	SOURCES
		${JAVA_FILES}
	OUTPUT_DIR
		"sdk/api/jvm"
	OUTPUT_NAME
		metaffi.api
)
set(metaffi.api metaffi.api PARENT_SCOPE)
add_dependencies(metaffi.api metaffi.api.accessor)

add_custom_target(api_jvm
	DEPENDS metaffi.api metaffi.api.accessor
)
set(api_jvm api_jvm PARENT_SCOPE)
