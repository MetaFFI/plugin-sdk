#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "cdts_jvm_serializer.h"
#include "runtime_id.h"
#include <runtime/xllr_capi_loader.h>
#include <utils/env_utils.h>
#include <jni.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace metaffi::utils;

// Global JVM instance for tests
static JavaVM* g_jvm = nullptr;
static JNIEnv* g_env = nullptr;

namespace
{
	using JNI_CreateJavaVM_t = jint (JNICALL *)(JavaVM**, void**, void*);

#ifdef _WIN32
	HMODULE g_libjvm_handle = nullptr;
#else
	void* g_libjvm_handle = nullptr;
#endif
	JNI_CreateJavaVM_t g_jni_create_java_vm = nullptr;

	std::filesystem::path libjvm_from_java_home(const std::filesystem::path& java_home)
	{
#ifdef _WIN32
		return java_home / "bin" / "server" / "jvm.dll";
#else
		return java_home / "lib" / "server" / "libjvm.so";
#endif
	}

	std::filesystem::path java_home_from_java_executable(const std::filesystem::path& java_executable)
	{
		auto exe_dir = java_executable.parent_path();
		if(exe_dir.filename() == "bin")
		{
			return exe_dir.parent_path();
		}
		return {};
	}

	std::filesystem::path normalize_configured_path(std::string configured_path)
	{
		if(configured_path.size() >= 2 &&
		   ((configured_path.front() == '"' && configured_path.back() == '"') ||
		    (configured_path.front() == '\'' && configured_path.back() == '\'')))
		{
			configured_path = configured_path.substr(1, configured_path.size() - 2);
		}

		return std::filesystem::path(configured_path);
	}

	std::filesystem::path resolve_libjvm_path()
	{
		std::vector<std::filesystem::path> candidates;

		const std::string java_home = get_env_var("JAVA_HOME");
		if(!java_home.empty())
		{
			candidates.emplace_back(libjvm_from_java_home(java_home));
		}

		const std::string jdk_home = get_env_var("JDK_HOME");
		if(!jdk_home.empty())
		{
			candidates.emplace_back(libjvm_from_java_home(jdk_home));
		}

#ifdef METAFFI_DEFAULT_JAVA_EXECUTABLE
		{
			const std::filesystem::path default_java_executable = normalize_configured_path(METAFFI_DEFAULT_JAVA_EXECUTABLE);
			const auto home = java_home_from_java_executable(default_java_executable);
			if(!home.empty())
			{
				candidates.emplace_back(libjvm_from_java_home(home));
			}
		}
#endif

#ifdef METAFFI_DEFAULT_LIBJVM_PATH
		{
			std::filesystem::path default_libjvm = normalize_configured_path(METAFFI_DEFAULT_LIBJVM_PATH);
#ifdef _WIN32
			if(default_libjvm.extension() == ".lib")
			{
				// FindJNI returns jvm.lib on Windows; convert to runtime jvm.dll path.
				auto root = default_libjvm.parent_path().parent_path();
				default_libjvm = root / "bin" / "server" / "jvm.dll";
			}
#endif
			candidates.emplace_back(default_libjvm);
		}
#endif

		for(const auto& candidate : candidates)
		{
			if(!candidate.empty() && std::filesystem::exists(candidate))
			{
				return candidate;
			}
		}

		throw std::runtime_error("Could not resolve libjvm path (checked JAVA_HOME/JDK_HOME and CMake-provided defaults)");
	}

	void ensure_jni_create_java_vm_loaded()
	{
		if(g_jni_create_java_vm)
		{
			return;
		}

		const auto libjvm_path = resolve_libjvm_path();

#ifdef _WIN32
		const auto server_dir = libjvm_path.parent_path();
		const auto bin_dir = server_dir.parent_path();

		using SetDefaultDllDirectories_t = BOOL (WINAPI *)(DWORD);
		using AddDllDirectory_t = DLL_DIRECTORY_COOKIE (WINAPI *)(PCWSTR);
		auto kernel32 = GetModuleHandleW(L"kernel32.dll");
		auto set_default_dll_directories = reinterpret_cast<SetDefaultDllDirectories_t>(
			GetProcAddress(kernel32, "SetDefaultDllDirectories"));
		auto add_dll_directory = reinterpret_cast<AddDllDirectory_t>(
			GetProcAddress(kernel32, "AddDllDirectory"));

		if(set_default_dll_directories)
		{
			set_default_dll_directories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		}
		if(add_dll_directory)
		{
			add_dll_directory(bin_dir.wstring().c_str());
			add_dll_directory(server_dir.wstring().c_str());
		}

		g_libjvm_handle = LoadLibraryExW(L"jvm.dll", nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		if(!g_libjvm_handle)
		{
			g_libjvm_handle = LoadLibraryW(libjvm_path.wstring().c_str());
		}
		if(!g_libjvm_handle)
		{
			throw std::runtime_error("Failed to load jvm.dll");
		}

		g_jni_create_java_vm = reinterpret_cast<JNI_CreateJavaVM_t>(GetProcAddress(g_libjvm_handle, "JNI_CreateJavaVM"));
		if(!g_jni_create_java_vm)
		{
			throw std::runtime_error("Failed to resolve JNI_CreateJavaVM from jvm.dll");
		}
#else
		dlerror();
		g_libjvm_handle = dlopen(libjvm_path.string().c_str(), RTLD_NOW | RTLD_GLOBAL);
		const char* load_err = dlerror();
		if(!g_libjvm_handle || load_err)
		{
			throw std::runtime_error(std::string("Failed to load libjvm.so: ") + (load_err ? load_err : "unknown error"));
		}

		dlerror();
		g_jni_create_java_vm = reinterpret_cast<JNI_CreateJavaVM_t>(dlsym(g_libjvm_handle, "JNI_CreateJavaVM"));
		const char* symbol_err = dlerror();
		if(!g_jni_create_java_vm || symbol_err)
		{
			throw std::runtime_error(std::string("Failed to resolve JNI_CreateJavaVM from libjvm.so: ") + (symbol_err ? symbol_err : "unknown error"));
		}
#endif
	}
}

// JVM initialization fixture
struct JVMFixture
{
	JVMFixture()
	{
		if (g_jvm == nullptr)
		{
			std::string metaffi_home = get_env_var("METAFFI_HOME");
			if(metaffi_home.empty())
			{
				throw std::runtime_error("METAFFI_HOME environment variable is not set");
			}

			std::filesystem::path api_jar = std::filesystem::path(metaffi_home) / "jvm" / "api" / "metaffi.api.jar";
			if(!std::filesystem::exists(api_jar))
			{
				throw std::runtime_error("Missing metaffi.api.jar at " + api_jar.string());
			}

			char sep = ';';
#ifndef _WIN32
			sep = ':';
#endif
			std::string classpath = std::string("-Djava.class.path=.") + sep + api_jar.generic_string();

			JavaVMInitArgs vm_args;
			JavaVMOption options[1];
			options[0].optionString = const_cast<char*>(classpath.c_str());

			vm_args.version = JNI_VERSION_1_8;
			vm_args.nOptions = 1;
			vm_args.options = options;
			vm_args.ignoreUnrecognized = JNI_FALSE;

			ensure_jni_create_java_vm_loaded();
			jint result = g_jni_create_java_vm(&g_jvm, (void**)&g_env, &vm_args);
			if (result != JNI_OK)
			{
				throw std::runtime_error("Failed to create JVM");
			}
		}
	}

	~JVMFixture()
	{
		// Keep JVM alive for all tests
	}
};

int main(int argc, char** argv)
{
	try
	{
		JVMFixture jvm_fixture;
		doctest::Context context;
		context.applyCommandLine(argc, argv);
		int res = context.run();
		if(context.shouldExit())
		{
			return res;
		}
		return res;
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	catch(...)
	{
		std::cerr << "Unknown error during JVM fixture initialization" << std::endl;
		return 1;
	}
}

TEST_SUITE("CDTS JVM Serializer")
{
	//--------------------------------------------------------------------
	// Primitive Types - Round-Trip Tests
	//--------------------------------------------------------------------

	TEST_CASE("Serialize and deserialize int8")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jbyte original = -42;
		ser.add(original, metaffi_int8_type);

		ser.reset();
		jbyte extracted = ser.extract_byte();
		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize int16")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jshort original = -12345;
		ser.add(original, metaffi_int16_type);

		ser.reset();
		jshort extracted = ser.extract_short();
		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize int32")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jint original = -123456789;
		ser.add(original, metaffi_int32_type);

		ser.reset();
		jint extracted = ser.extract_int();
		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize int64")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jlong original = -1234567890123456LL;
		ser.add(original, metaffi_int64_type);

		ser.reset();
		jlong extracted = ser.extract_long();
		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize uint8")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jbyte original = (jbyte)255; // Unsigned cast
		ser.add(original, metaffi_uint8_type);

		ser.reset();
		jbyte extracted = ser.extract_byte();
		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize uint16")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jshort original = (jshort)65535; // Unsigned cast
		ser.add(original, metaffi_uint16_type);

		ser.reset();
		jshort extracted = ser.extract_short();
		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize uint32")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jint original = (jint)4294967295U; // Unsigned cast
		ser.add(original, metaffi_uint32_type);

		ser.reset();
		jint extracted = ser.extract_int();
		CHECK((uint32_t)extracted == (uint32_t)original);
	}

	TEST_CASE("Serialize and deserialize uint64")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jlong original = (jlong)18446744073709551615ULL; // Unsigned cast
		ser.add(original, metaffi_uint64_type);

		ser.reset();
		jlong extracted = ser.extract_long();
		CHECK((uint64_t)extracted == (uint64_t)original);
	}

	TEST_CASE("Serialize and deserialize float32")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jfloat original = 3.14159f;
		ser.add(original, metaffi_float32_type);

		ser.reset();
		jfloat extracted = ser.extract_float();
		CHECK(extracted == doctest::Approx(original));
	}

	TEST_CASE("Serialize and deserialize float64")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jdouble original = 2.71828182845904523536;
		ser.add(original, metaffi_float64_type);

		ser.reset();
		jdouble extracted = ser.extract_double();
		CHECK(extracted == doctest::Approx(original));
	}

	TEST_CASE("Serialize and deserialize bool")
	{
		cdts data(2);
		cdts_jvm_serializer ser(g_env, data);

		ser.add(static_cast<jboolean>(JNI_TRUE), metaffi_bool_type);
		ser.add(static_cast<jboolean>(JNI_FALSE), metaffi_bool_type);

		ser.reset();
		CHECK(ser.extract_boolean() == JNI_TRUE);
		CHECK(ser.extract_boolean() == JNI_FALSE);
	}

	TEST_CASE("Serialize and deserialize char8")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jchar original = 'A'; // ASCII value 65
		ser.add(original, metaffi_char8_type);

		ser.reset();
		jchar extracted = ser.extract_char();
		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize char16")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jchar original = 0x4E2D; // Chinese character '中'
		ser.add(original, metaffi_char16_type);

		ser.reset();
		jchar extracted = ser.extract_char();
		CHECK(extracted == original);
	}

	//--------------------------------------------------------------------
	// String Types
	//--------------------------------------------------------------------

	TEST_CASE("Serialize and deserialize string8 (UTF-8)")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create Java string
		jstring original = g_env->NewStringUTF("Hello, MetaFFI!");
		ser << original;

		ser.reset();
		jstring extracted = ser.extract_string();

		// Compare strings
		const char* original_chars = g_env->GetStringUTFChars(original, nullptr);
		const char* extracted_chars = g_env->GetStringUTFChars(extracted, nullptr);
		CHECK(std::string(original_chars) == std::string(extracted_chars));

		g_env->ReleaseStringUTFChars(original, original_chars);
		g_env->ReleaseStringUTFChars(extracted, extracted_chars);
		g_env->DeleteLocalRef(original);
	}

	TEST_CASE("Serialize and deserialize empty string")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jstring original = g_env->NewStringUTF("");
		ser << original;

		ser.reset();
		jstring extracted = ser.extract_string();

		const char* extracted_chars = g_env->GetStringUTFChars(extracted, nullptr);
		CHECK(std::string(extracted_chars) == "");
		g_env->ReleaseStringUTFChars(extracted, extracted_chars);
		g_env->DeleteLocalRef(original);
	}

	TEST_CASE("Serialize and deserialize string16 (UTF-16)")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create Java string with UTF-16 characters (Hebrew: "מה שלומך")
		const jchar utf16_chars[] = {0x05DE, 0x05D4, 0x0020, 0x05E9, 0x05DC, 0x05D5, 0x05DE, 0x05DA, 0};
		jstring original = g_env->NewString(utf16_chars, 8);

		// Manually set as string16 in CDTS (since jstring auto-detects as string8)
		data[0].set_string((const char16_t*)utf16_chars, true);
		data[0].type = metaffi_string16_type;

		ser.reset();
		jstring extracted = ser.extract_string();

		// Compare strings
		jsize orig_len = g_env->GetStringLength(original);
		jsize extr_len = g_env->GetStringLength(extracted);
		CHECK(orig_len == extr_len);

		const jchar* orig_chars = g_env->GetStringChars(original, nullptr);
		const jchar* extr_chars = g_env->GetStringChars(extracted, nullptr);
		bool equal = true;
		for (jsize i = 0; i < orig_len; i++)
		{
			if (orig_chars[i] != extr_chars[i])
			{
				equal = false;
				break;
			}
		}
		CHECK(equal == true);

		g_env->ReleaseStringChars(original, orig_chars);
		g_env->ReleaseStringChars(extracted, extr_chars);
		g_env->DeleteLocalRef(original);
	}

	TEST_CASE("Serialize and deserialize string32 (UTF-32)")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create UTF-32 string (emoji: 🚀)
		const char32_t utf32_chars[] = {U'🚀', U' ', U'H', U'e', U'l', U'l', U'o', 0};
		data[0].set_string(utf32_chars, true);
		data[0].type = metaffi_string32_type;

		ser.reset();
		jstring extracted = ser.extract_string();

		// Verify it was converted to jstring (UTF-16)
		CHECK(extracted != nullptr);
		jsize len = g_env->GetStringLength(extracted);
		CHECK(len > 0);

		g_env->DeleteLocalRef(extracted);
	}

	//--------------------------------------------------------------------
	// Multiple Values
	//--------------------------------------------------------------------

	TEST_CASE("Serialize and deserialize multiple primitives")
	{
		cdts data(5);
		cdts_jvm_serializer ser(g_env, data);

		ser.add((jint)42, metaffi_int32_type);
		ser.add((jfloat)3.14f, metaffi_float32_type);
		ser.add((jdouble)2.71828, metaffi_float64_type);
		ser.add(static_cast<jboolean>(JNI_TRUE), metaffi_bool_type);
		ser.add((jlong)999LL, metaffi_int64_type);

		ser.reset();
		CHECK(ser.extract_int() == 42);
		CHECK(ser.extract_float() == doctest::Approx(3.14f));
		CHECK(ser.extract_double() == doctest::Approx(2.71828));
		CHECK(ser.extract_boolean() == JNI_TRUE);
		CHECK(ser.extract_long() == 999LL);
	}

	TEST_CASE("Mixed types serialization/deserialization")
	{
		cdts data(6);
		cdts_jvm_serializer ser(g_env, data);

		// Serialize diverse types
		ser.add((jbyte)10, metaffi_int8_type);
		jstring str = g_env->NewStringUTF("hello");
		ser << str;
		ser.add((jfloat)3.14f, metaffi_float32_type);
		ser.add(static_cast<jboolean>(JNI_TRUE), metaffi_bool_type);
		ser.add((jlong)9999LL, metaffi_uint64_type);

		// Add string16 manually
		const jchar utf16_chars[] = {0x05DE, 0x05D4, 0}; // Hebrew "מה"
		data[5].set_string((const char16_t*)utf16_chars, true);
		data[5].type = metaffi_string16_type;

		ser.reset();
		CHECK(ser.extract_byte() == 10);
		jstring extracted_str = ser.extract_string();
		const char* str_chars = g_env->GetStringUTFChars(extracted_str, nullptr);
		CHECK(std::string(str_chars) == "hello");
		g_env->ReleaseStringUTFChars(extracted_str, str_chars);
		CHECK(ser.extract_float() == doctest::Approx(3.14f));
		CHECK(ser.extract_boolean() == JNI_TRUE);
		CHECK((uint64_t)ser.extract_long() == 9999ULL);
		jstring extracted_str16 = ser.extract_string();
		CHECK(extracted_str16 != nullptr);
		g_env->DeleteLocalRef(extracted_str16);

		g_env->DeleteLocalRef(str);
	}

	TEST_CASE("Chaining operations")
	{
		cdts data(3);
		cdts_jvm_serializer ser(g_env, data);

		// Chain serialization
		ser.add((jint)1, metaffi_int32_type)
			.add((jint)2, metaffi_int32_type)
			.add((jint)3, metaffi_int32_type);

		ser.reset();
		// Chain deserialization
		jint val1 = ser.extract_int();
		jint val2 = ser.extract_int();
		jint val3 = ser.extract_int();

		CHECK(val1 == 1);
		CHECK(val2 == 2);
		CHECK(val3 == 3);
	}

	//--------------------------------------------------------------------
	// Array Tests
	//--------------------------------------------------------------------

	TEST_CASE("Serialize and deserialize 1D int array")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create Java int array
		jintArray arr = g_env->NewIntArray(5);
		jint values[] = {1, 2, 3, 4, 5};
		g_env->SetIntArrayRegion(arr, 0, 5, values);

		ser << (jobject)arr;

		ser.reset();
		jintArray extracted = (jintArray)ser.extract_array();

		CHECK(g_env->GetArrayLength(extracted) == 5);
		jint* elements = g_env->GetIntArrayElements(extracted, nullptr);
		for (int i = 0; i < 5; i++)
		{
			CHECK(elements[i] == values[i]);
		}
		g_env->ReleaseIntArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Serialize and deserialize 1D double array")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jdoubleArray arr = g_env->NewDoubleArray(3);
		jdouble values[] = {1.1, 2.2, 3.3};
		g_env->SetDoubleArrayRegion(arr, 0, 3, values);

		ser << (jobject)arr;

		ser.reset();
		jdoubleArray extracted = (jdoubleArray)ser.extract_array();

		CHECK(g_env->GetArrayLength(extracted) == 3);
		jdouble* elements = g_env->GetDoubleArrayElements(extracted, nullptr);
		for (int i = 0; i < 3; i++)
		{
			CHECK(elements[i] == doctest::Approx(values[i]));
		}
		g_env->ReleaseDoubleArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Serialize and deserialize empty array")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jintArray arr = g_env->NewIntArray(0);
		ser << (jobject)arr;

		ser.reset();
		jintArray extracted = (jintArray)ser.extract_array();

		// Empty array returns nullptr
		CHECK((extracted == nullptr || g_env->GetArrayLength(extracted) == 0));
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Serialize and deserialize 2D array")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create 2D int array [[1, 2, 3], [4, 5, 6]]
		jintArray row1 = g_env->NewIntArray(3);
		jintArray row2 = g_env->NewIntArray(3);
		jint values1[] = {1, 2, 3};
		jint values2[] = {4, 5, 6};
		g_env->SetIntArrayRegion(row1, 0, 3, values1);
		g_env->SetIntArrayRegion(row2, 0, 3, values2);

		jobjectArray arr2d = g_env->NewObjectArray(2, g_env->FindClass("[I"), nullptr);
		g_env->SetObjectArrayElement(arr2d, 0, row1);
		g_env->SetObjectArrayElement(arr2d, 1, row2);

		// Serialize the 2D array
		ser << (jobject)arr2d;

		// Verify CDTS structure
		CHECK((data[0].type & metaffi_array_type) != 0);
		cdts* outer = data[0].cdt_val.array_val;
		CHECK(outer != nullptr);
		CHECK(outer->length == 2);

		// Each element should be an array
		CHECK(((*outer)[0].type & metaffi_array_type) != 0);
		CHECK(((*outer)[1].type & metaffi_array_type) != 0);

		// Check first row: [1, 2, 3]
		cdts* row0 = (*outer)[0].cdt_val.array_val;
		CHECK(row0 != nullptr);
		CHECK(row0->length == 3);
		CHECK((*row0)[0].cdt_val.int32_val == 1);
		CHECK((*row0)[1].cdt_val.int32_val == 2);
		CHECK((*row0)[2].cdt_val.int32_val == 3);

		// Check second row: [4, 5, 6]
		cdts* row1_cdts = (*outer)[1].cdt_val.array_val;
		CHECK(row1_cdts != nullptr);
		CHECK(row1_cdts->length == 3);
		CHECK((*row1_cdts)[0].cdt_val.int32_val == 4);
		CHECK((*row1_cdts)[1].cdt_val.int32_val == 5);
		CHECK((*row1_cdts)[2].cdt_val.int32_val == 6);

		// Deserialize and verify
		ser.reset();
		jarray extracted = ser.extract_array();
		CHECK(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 2);

		// Verify extracted values
		jobjectArray extracted2d = (jobjectArray)extracted;
		jintArray extractedRow0 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 0);
		jintArray extractedRow1 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 1);

		CHECK(g_env->GetArrayLength(extractedRow0) == 3);
		CHECK(g_env->GetArrayLength(extractedRow1) == 3);

		jint* elem0 = g_env->GetIntArrayElements(extractedRow0, nullptr);
		CHECK(elem0[0] == 1);
		CHECK(elem0[1] == 2);
		CHECK(elem0[2] == 3);
		g_env->ReleaseIntArrayElements(extractedRow0, elem0, JNI_ABORT);

		jint* elem1 = g_env->GetIntArrayElements(extractedRow1, nullptr);
		CHECK(elem1[0] == 4);
		CHECK(elem1[1] == 5);
		CHECK(elem1[2] == 6);
		g_env->ReleaseIntArrayElements(extractedRow1, elem1, JNI_ABORT);

		g_env->DeleteLocalRef(extractedRow0);
		g_env->DeleteLocalRef(extractedRow1);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(row1);
		g_env->DeleteLocalRef(row2);
		g_env->DeleteLocalRef(arr2d);
	}

	TEST_CASE("Serialize and deserialize 3D array")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create 3D int array [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
		jintArray arr1_1 = g_env->NewIntArray(2);
		jintArray arr1_2 = g_env->NewIntArray(2);
		jintArray arr2_1 = g_env->NewIntArray(2);
		jintArray arr2_2 = g_env->NewIntArray(2);
		jint v1_1[] = {1, 2};
		jint v1_2[] = {3, 4};
		jint v2_1[] = {5, 6};
		jint v2_2[] = {7, 8};
		g_env->SetIntArrayRegion(arr1_1, 0, 2, v1_1);
		g_env->SetIntArrayRegion(arr1_2, 0, 2, v1_2);
		g_env->SetIntArrayRegion(arr2_1, 0, 2, v2_1);
		g_env->SetIntArrayRegion(arr2_2, 0, 2, v2_2);

		jobjectArray arr2d_1 = g_env->NewObjectArray(2, g_env->FindClass("[I"), nullptr);
		jobjectArray arr2d_2 = g_env->NewObjectArray(2, g_env->FindClass("[I"), nullptr);
		g_env->SetObjectArrayElement(arr2d_1, 0, arr1_1);
		g_env->SetObjectArrayElement(arr2d_1, 1, arr1_2);
		g_env->SetObjectArrayElement(arr2d_2, 0, arr2_1);
		g_env->SetObjectArrayElement(arr2d_2, 1, arr2_2);

		jobjectArray arr3d = g_env->NewObjectArray(2, g_env->FindClass("[[I"), nullptr);
		g_env->SetObjectArrayElement(arr3d, 0, arr2d_1);
		g_env->SetObjectArrayElement(arr3d, 1, arr2d_2);

		// Serialize the 3D array
		ser << (jobject)arr3d;

		// Verify CDTS structure - 3D means 3 levels of nesting
		CHECK((data[0].type & metaffi_array_type) != 0);
		cdts* outer = data[0].cdt_val.array_val;
		CHECK(outer != nullptr);
		CHECK(outer->length == 2);

		// First 2D array [[1,2],[3,4]]
		CHECK(((*outer)[0].type & metaffi_array_type) != 0);
		cdts* arr2d_0 = (*outer)[0].cdt_val.array_val;
		CHECK(arr2d_0 != nullptr);
		CHECK(arr2d_0->length == 2);

		// [1,2]
		CHECK(((*arr2d_0)[0].type & metaffi_array_type) != 0);
		cdts* arr1d_0_0 = (*arr2d_0)[0].cdt_val.array_val;
		CHECK(arr1d_0_0 != nullptr);
		CHECK(arr1d_0_0->length == 2);
		CHECK((*arr1d_0_0)[0].cdt_val.int32_val == 1);
		CHECK((*arr1d_0_0)[1].cdt_val.int32_val == 2);

		// [3,4]
		CHECK(((*arr2d_0)[1].type & metaffi_array_type) != 0);
		cdts* arr1d_0_1 = (*arr2d_0)[1].cdt_val.array_val;
		CHECK(arr1d_0_1 != nullptr);
		CHECK(arr1d_0_1->length == 2);
		CHECK((*arr1d_0_1)[0].cdt_val.int32_val == 3);
		CHECK((*arr1d_0_1)[1].cdt_val.int32_val == 4);

		// Cleanup
		g_env->DeleteLocalRef(arr1_1);
		g_env->DeleteLocalRef(arr1_2);
		g_env->DeleteLocalRef(arr2_1);
		g_env->DeleteLocalRef(arr2_2);
		g_env->DeleteLocalRef(arr2d_1);
		g_env->DeleteLocalRef(arr2d_2);
		g_env->DeleteLocalRef(arr3d);
	}

	TEST_CASE("Serialize and deserialize ragged 2D array")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create ragged 2D array [[1], [2, 3], [4, 5, 6]]
		jintArray row1 = g_env->NewIntArray(1);
		jintArray row2 = g_env->NewIntArray(2);
		jintArray row3 = g_env->NewIntArray(3);
		jint v1[] = {1};
		jint v2[] = {2, 3};
		jint v3[] = {4, 5, 6};
		g_env->SetIntArrayRegion(row1, 0, 1, v1);
		g_env->SetIntArrayRegion(row2, 0, 2, v2);
		g_env->SetIntArrayRegion(row3, 0, 3, v3);

		jobjectArray arr2d = g_env->NewObjectArray(3, g_env->FindClass("[I"), nullptr);
		g_env->SetObjectArrayElement(arr2d, 0, row1);
		g_env->SetObjectArrayElement(arr2d, 1, row2);
		g_env->SetObjectArrayElement(arr2d, 2, row3);

		// Serialize the ragged 2D array
		ser << (jobject)arr2d;

		// Verify CDTS structure - outer array has 3 elements of different lengths
		CHECK((data[0].type & metaffi_array_type) != 0);
		cdts* outer = data[0].cdt_val.array_val;
		CHECK(outer != nullptr);
		CHECK(outer->length == 3);

		// Row 0: [1] - length 1
		CHECK(((*outer)[0].type & metaffi_array_type) != 0);
		cdts* r0 = (*outer)[0].cdt_val.array_val;
		CHECK(r0 != nullptr);
		CHECK(r0->length == 1);
		CHECK((*r0)[0].cdt_val.int32_val == 1);

		// Row 1: [2, 3] - length 2
		CHECK(((*outer)[1].type & metaffi_array_type) != 0);
		cdts* r1 = (*outer)[1].cdt_val.array_val;
		CHECK(r1 != nullptr);
		CHECK(r1->length == 2);
		CHECK((*r1)[0].cdt_val.int32_val == 2);
		CHECK((*r1)[1].cdt_val.int32_val == 3);

		// Row 2: [4, 5, 6] - length 3
		CHECK(((*outer)[2].type & metaffi_array_type) != 0);
		cdts* r2 = (*outer)[2].cdt_val.array_val;
		CHECK(r2 != nullptr);
		CHECK(r2->length == 3);
		CHECK((*r2)[0].cdt_val.int32_val == 4);
		CHECK((*r2)[1].cdt_val.int32_val == 5);
		CHECK((*r2)[2].cdt_val.int32_val == 6);

		// Deserialize and verify ragged structure preserved
		ser.reset();
		jarray extracted = ser.extract_array();
		CHECK(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);

		jobjectArray extracted2d = (jobjectArray)extracted;
		jintArray ex0 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 0);
		jintArray ex1 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 1);
		jintArray ex2 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 2);

		CHECK(g_env->GetArrayLength(ex0) == 1);
		CHECK(g_env->GetArrayLength(ex1) == 2);
		CHECK(g_env->GetArrayLength(ex2) == 3);

		g_env->DeleteLocalRef(ex0);
		g_env->DeleteLocalRef(ex1);
		g_env->DeleteLocalRef(ex2);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(row1);
		g_env->DeleteLocalRef(row2);
		g_env->DeleteLocalRef(row3);
		g_env->DeleteLocalRef(arr2d);
	}

	//--------------------------------------------------------------------
	// Wrapper Object Tests (Auto-Detection)
	//--------------------------------------------------------------------

	TEST_CASE("Serialize and deserialize Integer wrapper")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create Integer object
		jclass integerClass = g_env->FindClass("java/lang/Integer");
		jmethodID constructor = g_env->GetMethodID(integerClass, "<init>", "(I)V");
		jobject integerObj = g_env->NewObject(integerClass, constructor, (jint)42);

		ser << integerObj;

		ser.reset();
		jint extracted = ser.extract_int();
		CHECK(extracted == 42);

		g_env->DeleteLocalRef(integerObj);
		g_env->DeleteLocalRef(integerClass);
	}

	TEST_CASE("Serialize and deserialize Long wrapper")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jclass longClass = g_env->FindClass("java/lang/Long");
		jmethodID constructor = g_env->GetMethodID(longClass, "<init>", "(J)V");
		jobject longObj = g_env->NewObject(longClass, constructor, (jlong)999999LL);

		ser << longObj;

		ser.reset();
		jlong extracted = ser.extract_long();
		CHECK(extracted == 999999LL);

		g_env->DeleteLocalRef(longObj);
		g_env->DeleteLocalRef(longClass);
	}

	TEST_CASE("Serialize and deserialize Double wrapper")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jclass doubleClass = g_env->FindClass("java/lang/Double");
		jmethodID constructor = g_env->GetMethodID(doubleClass, "<init>", "(D)V");
		jobject doubleObj = g_env->NewObject(doubleClass, constructor, (jdouble)2.71828);

		ser << doubleObj;

		ser.reset();
		jdouble extracted = ser.extract_double();
		CHECK(extracted == doctest::Approx(2.71828));

		g_env->DeleteLocalRef(doubleObj);
		g_env->DeleteLocalRef(doubleClass);
	}

	//--------------------------------------------------------------------
	// Special Values
	//--------------------------------------------------------------------

	TEST_CASE("Serialize and deserialize null")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		ser.null();

		ser.reset();
		CHECK(ser.is_null() == true);
		CHECK(ser.peek_type() == metaffi_null_type);
	}

	TEST_CASE("Handle serialization")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create a Java object to store as handle (use Object class, not a wrapper)
		jclass objectClass = g_env->FindClass("java/lang/Object");
		jobject original = g_env->AllocObject(objectClass);

		// Manually set as handle in CDTS (since Object will be detected as handle by operator<<)
		data[0].type = metaffi_handle_type;
		data[0].cdt_val.handle_val = new cdt_metaffi_handle();
		data[0].cdt_val.handle_val->handle = g_env->NewGlobalRef(original);
		data[0].cdt_val.handle_val->runtime_id = JVM_RUNTIME_ID;
		data[0].free_required = true;

		ser.reset();
		jobject extracted = ser.extract_handle();

		// Verify it's the same object (global reference)
		CHECK(extracted != nullptr);
		CHECK(extracted == data[0].cdt_val.handle_val->handle);

		g_env->DeleteLocalRef(original);
		g_env->DeleteLocalRef(objectClass);
		// Note: extracted is a global ref, don't delete it
	}

	TEST_CASE("Callable serialization")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create callable with parameter and return types
		metaffi_type param_types[] = {metaffi_int32_type, metaffi_string8_type};
		metaffi_type retval_types[] = {metaffi_float64_type};

		// Allocate arrays with xllr_alloc
		metaffi_type* param_types_ptr = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 2);
		metaffi_type* retval_types_ptr = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 1);
		memcpy(param_types_ptr, param_types, sizeof(metaffi_type) * 2);
		memcpy(retval_types_ptr, retval_types, sizeof(metaffi_type) * 1);

		// Manually set in CDTS (since there's no direct callable serialization method)
		data[0].type = metaffi_callable_type;
		data[0].cdt_val.callable_val = new cdt_metaffi_callable();
		data[0].cdt_val.callable_val->val = (void*)0x1234ABCD; // Stubbed function pointer
		data[0].cdt_val.callable_val->parameters_types = param_types_ptr;
		data[0].cdt_val.callable_val->params_types_length = 2;
		data[0].cdt_val.callable_val->retval_types = retval_types_ptr;
		data[0].cdt_val.callable_val->retval_types_length = 1;
		data[0].free_required = true;

		ser.reset();

		// Verify callable data directly (extract_handle for callables may not return useful jobject)
		CHECK(data[0].cdt_val.callable_val->val == (void*)0x1234ABCD);
		CHECK(data[0].cdt_val.callable_val->params_types_length == 2);
		CHECK(data[0].cdt_val.callable_val->retval_types_length == 1);
		CHECK(data[0].cdt_val.callable_val->parameters_types[0] == metaffi_int32_type);
		CHECK(data[0].cdt_val.callable_val->parameters_types[1] == metaffi_string8_type);
		CHECK(data[0].cdt_val.callable_val->retval_types[0] == metaffi_float64_type);

		// Note: Memory will be freed when CDTS is destroyed
	}

	//--------------------------------------------------------------------
	// Type Introspection
	//--------------------------------------------------------------------

	TEST_CASE("Type query with peek_type")
	{
		cdts data(3);
		cdts_jvm_serializer ser(g_env, data);

		ser.add((jint)42, metaffi_int32_type);
		jstring str = g_env->NewStringUTF("hello");
		ser << str;
		ser.add((jdouble)3.14, metaffi_float64_type);

		ser.reset();

		// Peek without consuming
		CHECK(ser.peek_type() == metaffi_int32_type);
		CHECK(ser.extract_int() == 42);

		CHECK(ser.peek_type() == metaffi_string8_type);
		jstring extracted_str = ser.extract_string();

		CHECK(ser.peek_type() == metaffi_float64_type);
		CHECK(ser.extract_double() == doctest::Approx(3.14));

		g_env->DeleteLocalRef(str);
	}

	//--------------------------------------------------------------------
	// Type Preservation Tests
	//--------------------------------------------------------------------

	TEST_CASE("Verify CDTS stores correct integer types after serialization")
	{
		cdts data(8);
		cdts_jvm_serializer ser(g_env, data);

		// Serialize different integer types
		ser.add((jbyte)-10, metaffi_int8_type);
		ser.add((jshort)-1000, metaffi_int16_type);
		ser.add((jint)-100000, metaffi_int32_type);
		ser.add((jlong)-10000000LL, metaffi_int64_type);
		ser.add((jbyte)10, metaffi_uint8_type);
		ser.add((jshort)1000, metaffi_uint16_type);
		ser.add((jint)100000, metaffi_uint32_type);
		ser.add((jlong)10000000LL, metaffi_uint64_type);

		// Verify CDTS has correct types stored
		CHECK(data[0].type == metaffi_int8_type);
		CHECK(data[1].type == metaffi_int16_type);
		CHECK(data[2].type == metaffi_int32_type);
		CHECK(data[3].type == metaffi_int64_type);
		CHECK(data[4].type == metaffi_uint8_type);
		CHECK(data[5].type == metaffi_uint16_type);
		CHECK(data[6].type == metaffi_uint32_type);
		CHECK(data[7].type == metaffi_uint64_type);
	}

	TEST_CASE("Verify CDTS stores correct float types after serialization")
	{
		cdts data(2);
		cdts_jvm_serializer ser(g_env, data);

		// Serialize different float types
		ser.add((jfloat)3.14f, metaffi_float32_type);
		ser.add((jdouble)2.71828, metaffi_float64_type);

		// Verify CDTS has correct types stored
		CHECK(data[0].type == metaffi_float32_type);
		CHECK(data[1].type == metaffi_float64_type);
	}

	TEST_CASE("Verify CDTS stores correct types for mixed primitives")
	{
		cdts data(5);
		cdts_jvm_serializer ser(g_env, data);

		// Serialize mixed types
		ser.add((jint)42, metaffi_int32_type);
		ser.add((jfloat)3.14f, metaffi_float32_type);
		ser.add((jdouble)2.71828, metaffi_float64_type);
		ser.add(static_cast<jboolean>(JNI_TRUE), metaffi_bool_type);
		ser.add((jshort)1000, metaffi_int16_type);

		// Verify types
		CHECK(data[0].type == metaffi_int32_type);
		CHECK(data[1].type == metaffi_float32_type);
		CHECK(data[2].type == metaffi_float64_type);
		CHECK(data[3].type == metaffi_bool_type);
		CHECK(data[4].type == metaffi_int16_type);
	}

	//--------------------------------------------------------------------
	// Utility Methods
	//--------------------------------------------------------------------

	TEST_CASE("Utility methods")
	{
		cdts data(5);
		cdts_jvm_serializer ser(g_env, data);

		// Initial state
		CHECK(ser.get_index() == 0);
		CHECK(ser.size() == 5);
		CHECK(ser.has_more() == true);

		// Add values
		ser.add((jint)1, metaffi_int32_type);
		ser.add((jint)2, metaffi_int32_type);
		ser.add((jint)3, metaffi_int32_type);

		CHECK(ser.get_index() == 3);
		CHECK(ser.has_more() == true);

		// Reset
		ser.reset();
		CHECK(ser.get_index() == 0);

		// Set index
		ser.set_index(2);
		CHECK(ser.get_index() == 2);
	}

	//--------------------------------------------------------------------
	// Error Handling
	//--------------------------------------------------------------------

	TEST_CASE("Error: Type mismatch on deserialization")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		ser.add((jint)42, metaffi_int32_type);

		ser.reset();
		CHECK_THROWS_AS(ser.extract_string(), std::runtime_error);
	}

	TEST_CASE("Error: Bounds violation on serialization")
	{
		cdts data(2);
		cdts_jvm_serializer ser(g_env, data);

		ser.add((jint)1, metaffi_int32_type);
		ser.add((jint)2, metaffi_int32_type);

		CHECK_THROWS_AS(ser.add((jint)3, metaffi_int32_type), std::out_of_range);
	}

	TEST_CASE("Error: Bounds violation on deserialization")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		ser.add((jint)42, metaffi_int32_type);

		ser.reset();
		ser.extract_int();

		CHECK_THROWS_AS(ser.extract_int(), std::out_of_range);
	}

	TEST_CASE("Error: Value out of range for char8")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jchar value = 300; // Too large for char8 (0-255)
		CHECK_THROWS_WITH(ser.add(value, metaffi_char8_type),
						doctest::Contains("out of range"));
	}

	TEST_CASE("Error: Peek type beyond bounds")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		ser.add((jint)42, metaffi_int32_type);

		// After adding one value, index is at 1
		// peek_type at index 1 should throw
		CHECK_THROWS_AS(ser.peek_type(), std::out_of_range);
	}

	//--------------------------------------------------------------------
	// Deserialization-Only Tests (Pre-filled CDTS)
	//--------------------------------------------------------------------

	TEST_CASE("Deserialize pre-filled CDTS with primitives")
	{
		// Manually create CDTS (simulating MetaFFI call)
		cdts data(3);
		data[0] = (metaffi_int32)42;
		data[1].set_string((const char8_t*)"test", true);
		data[2] = (metaffi_float64)3.14;

		// Deserialize only
		cdts_jvm_serializer deser(g_env, data);

		jint i = deser.extract_int();
		CHECK(i == 42);

		jstring s = deser.extract_string();
		const char* str_chars = g_env->GetStringUTFChars(s, nullptr);
		CHECK(std::string(str_chars) == "test");
		g_env->ReleaseStringUTFChars(s, str_chars);

		jdouble d = deser.extract_double();
		CHECK(d == doctest::Approx(3.14));
	}

	TEST_CASE("Deserialize pre-filled CDTS with all primitive types")
	{
		cdts data(8);

		// Manually fill CDTS
		data[0] = (metaffi_int8)-10;
		data[1] = (metaffi_int16)-1000;
		data[2] = (metaffi_int32)-100000;
		data[3] = (metaffi_int64)-10000000LL;
		data[4] = (metaffi_uint8)10;
		data[5] = (metaffi_uint16)1000;
		data[6] = (metaffi_uint32)100000;
		data[7] = (metaffi_uint64)10000000ULL;

		cdts_jvm_serializer deser(g_env, data);

		CHECK(deser.extract_byte() == -10);
		CHECK(deser.extract_short() == -1000);
		CHECK(deser.extract_int() == -100000);
		CHECK(deser.extract_long() == -10000000LL);
		CHECK((uint8_t)deser.extract_byte() == 10);
		CHECK((uint16_t)deser.extract_short() == 1000);
		CHECK((uint32_t)deser.extract_int() == 100000);
		CHECK((uint64_t)deser.extract_long() == 10000000ULL);
	}

	TEST_CASE("Deserialize pre-filled CDTS with strings")
	{
		// Manually create CDTS with strings
		cdts data(3);
		data[0].set_string((const char8_t*)"Hello UTF-8", true);
		data[0].type = metaffi_string8_type;
		data[1].set_string(u"Hello UTF-16", true);
		data[1].type = metaffi_string16_type;
		data[2].set_string(U"Hello UTF-32", true);
		data[2].type = metaffi_string32_type;

		// Deserialize
		cdts_jvm_serializer deser(g_env, data);

		jstring s8 = deser.extract_string();
		const char* s8_chars = g_env->GetStringUTFChars(s8, nullptr);
		CHECK(std::string(s8_chars) == "Hello UTF-8");
		g_env->ReleaseStringUTFChars(s8, s8_chars);

		jstring s16 = deser.extract_string();
		CHECK(s16 != nullptr);
		jsize s16_len = g_env->GetStringLength(s16);
		CHECK(s16_len > 0);
		g_env->DeleteLocalRef(s16);

		jstring s32 = deser.extract_string();
		CHECK(s32 != nullptr);
		g_env->DeleteLocalRef(s32);
	}

	TEST_CASE("Deserialize pre-filled CDTS with 1D array")
	{
		// Manually create 1D array: [10, 20, 30, 40, 50]
		cdts data(1);
		data[0].set_new_array(5, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr = static_cast<cdts&>(data[0]);
		arr[0] = (metaffi_int32)10;
		arr[1] = (metaffi_int32)20;
		arr[2] = (metaffi_int32)30;
		arr[3] = (metaffi_int32)40;
		arr[4] = (metaffi_int32)50;

		// Deserialize
		cdts_jvm_serializer deser(g_env, data);
		jintArray extracted = (jintArray)deser.extract_array();

		CHECK(g_env->GetArrayLength(extracted) == 5);
		jint* elements = g_env->GetIntArrayElements(extracted, nullptr);
		CHECK(elements[0] == 10);
		CHECK(elements[1] == 20);
		CHECK(elements[2] == 30);
		CHECK(elements[3] == 40);
		CHECK(elements[4] == 50);
		g_env->ReleaseIntArrayElements(extracted, elements, JNI_ABORT);
	}

	TEST_CASE("Deserialize pre-filled CDTS with 2D array")
	{
		// Manually create 2D array: [[1, 2, 3], [4, 5, 6]]
		cdts data(1);
		data[0].set_new_array(2, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr = static_cast<cdts&>(data[0]);

		arr[0].set_new_array(3, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& row0 = static_cast<cdts&>(arr[0]);
		row0[0] = (metaffi_int32)1;
		row0[1] = (metaffi_int32)2;
		row0[2] = (metaffi_int32)3;

		arr[1].set_new_array(3, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& row1 = static_cast<cdts&>(arr[1]);
		row1[0] = (metaffi_int32)4;
		row1[1] = (metaffi_int32)5;
		row1[2] = (metaffi_int32)6;

		// Deserialize
		cdts_jvm_serializer deser(g_env, data);
		jarray extracted = deser.extract_array();

		CHECK(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 2);

		jobjectArray extracted2d = (jobjectArray)extracted;
		jintArray extractedRow0 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 0);
		jintArray extractedRow1 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 1);

		CHECK(g_env->GetArrayLength(extractedRow0) == 3);
		CHECK(g_env->GetArrayLength(extractedRow1) == 3);

		jint* elem0 = g_env->GetIntArrayElements(extractedRow0, nullptr);
		CHECK(elem0[0] == 1);
		CHECK(elem0[1] == 2);
		CHECK(elem0[2] == 3);
		g_env->ReleaseIntArrayElements(extractedRow0, elem0, JNI_ABORT);

		jint* elem1 = g_env->GetIntArrayElements(extractedRow1, nullptr);
		CHECK(elem1[0] == 4);
		CHECK(elem1[1] == 5);
		CHECK(elem1[2] == 6);
		g_env->ReleaseIntArrayElements(extractedRow1, elem1, JNI_ABORT);

		g_env->DeleteLocalRef(extractedRow0);
		g_env->DeleteLocalRef(extractedRow1);
		g_env->DeleteLocalRef(extracted);
	}

	TEST_CASE("Deserialize pre-filled CDTS with 3D array")
	{
		// Manually create 3D array: [[[1, 2]], [[3, 4]]]
		cdts data(1);
		data[0].set_new_array(2, 3, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr3d = static_cast<cdts&>(data[0]);

		// First 2D element
		arr3d[0].set_new_array(1, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr2d_0 = static_cast<cdts&>(arr3d[0]);
		arr2d_0[0].set_new_array(2, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr1d_0 = static_cast<cdts&>(arr2d_0[0]);
		arr1d_0[0] = (metaffi_int32)1;
		arr1d_0[1] = (metaffi_int32)2;

		// Second 2D element
		arr3d[1].set_new_array(1, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr2d_1 = static_cast<cdts&>(arr3d[1]);
		arr2d_1[0].set_new_array(2, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr1d_1 = static_cast<cdts&>(arr2d_1[0]);
		arr1d_1[0] = (metaffi_int32)3;
		arr1d_1[1] = (metaffi_int32)4;

		// Deserialize
		cdts_jvm_serializer deser(g_env, data);
		jarray extracted = deser.extract_array();

		CHECK(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 2);

		// Verify structure: [[[1, 2]], [[3, 4]]]
		jobjectArray extracted3d = (jobjectArray)extracted;

		// First 2D element: [[1, 2]]
		jobjectArray ext2d_0 = (jobjectArray)g_env->GetObjectArrayElement(extracted3d, 0);
		CHECK(g_env->GetArrayLength(ext2d_0) == 1);

		jintArray ext1d_0 = (jintArray)g_env->GetObjectArrayElement(ext2d_0, 0);
		CHECK(g_env->GetArrayLength(ext1d_0) == 2);
		jint* elem0 = g_env->GetIntArrayElements(ext1d_0, nullptr);
		CHECK(elem0[0] == 1);
		CHECK(elem0[1] == 2);
		g_env->ReleaseIntArrayElements(ext1d_0, elem0, JNI_ABORT);

		// Second 2D element: [[3, 4]]
		jobjectArray ext2d_1 = (jobjectArray)g_env->GetObjectArrayElement(extracted3d, 1);
		CHECK(g_env->GetArrayLength(ext2d_1) == 1);

		jintArray ext1d_1 = (jintArray)g_env->GetObjectArrayElement(ext2d_1, 0);
		CHECK(g_env->GetArrayLength(ext1d_1) == 2);
		jint* elem1 = g_env->GetIntArrayElements(ext1d_1, nullptr);
		CHECK(elem1[0] == 3);
		CHECK(elem1[1] == 4);
		g_env->ReleaseIntArrayElements(ext1d_1, elem1, JNI_ABORT);

		g_env->DeleteLocalRef(ext1d_0);
		g_env->DeleteLocalRef(ext1d_1);
		g_env->DeleteLocalRef(ext2d_0);
		g_env->DeleteLocalRef(ext2d_1);
		g_env->DeleteLocalRef(extracted);
	}

	TEST_CASE("Deserialize pre-filled CDTS with ragged array")
	{
		// Manually create ragged array: [[1], [2, 3], [4, 5, 6]]
		cdts data(1);
		data[0].set_new_array(3, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr = static_cast<cdts&>(data[0]);

		arr[0].set_new_array(1, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr[0])[0] = (metaffi_int32)1;

		arr[1].set_new_array(2, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& row1 = static_cast<cdts&>(arr[1]);
		row1[0] = (metaffi_int32)2;
		row1[1] = (metaffi_int32)3;

		arr[2].set_new_array(3, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& row2 = static_cast<cdts&>(arr[2]);
		row2[0] = (metaffi_int32)4;
		row2[1] = (metaffi_int32)5;
		row2[2] = (metaffi_int32)6;

		// Deserialize
		cdts_jvm_serializer deser(g_env, data);
		jarray extracted = deser.extract_array();

		CHECK(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);

		jobjectArray extracted2d = (jobjectArray)extracted;

		// Row 0: [1] - length 1
		jintArray ex0 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 0);
		CHECK(g_env->GetArrayLength(ex0) == 1);
		jint* elem0 = g_env->GetIntArrayElements(ex0, nullptr);
		CHECK(elem0[0] == 1);
		g_env->ReleaseIntArrayElements(ex0, elem0, JNI_ABORT);

		// Row 1: [2, 3] - length 2
		jintArray ex1 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 1);
		CHECK(g_env->GetArrayLength(ex1) == 2);
		jint* elem1 = g_env->GetIntArrayElements(ex1, nullptr);
		CHECK(elem1[0] == 2);
		CHECK(elem1[1] == 3);
		g_env->ReleaseIntArrayElements(ex1, elem1, JNI_ABORT);

		// Row 2: [4, 5, 6] - length 3
		jintArray ex2 = (jintArray)g_env->GetObjectArrayElement(extracted2d, 2);
		CHECK(g_env->GetArrayLength(ex2) == 3);
		jint* elem2 = g_env->GetIntArrayElements(ex2, nullptr);
		CHECK(elem2[0] == 4);
		CHECK(elem2[1] == 5);
		CHECK(elem2[2] == 6);
		g_env->ReleaseIntArrayElements(ex2, elem2, JNI_ABORT);

		g_env->DeleteLocalRef(ex0);
		g_env->DeleteLocalRef(ex1);
		g_env->DeleteLocalRef(ex2);
		g_env->DeleteLocalRef(extracted);
	}

	TEST_CASE("Deserialize pre-filled CDTS with handle")
	{
		// Manually create handle
		cdts data(1);
		data[0].type = metaffi_handle_type;
		data[0].cdt_val.handle_val = new cdt_metaffi_handle();

		// Create a Java object and store as global ref
		jclass integerClass = g_env->FindClass("java/lang/Integer");
		jmethodID constructor = g_env->GetMethodID(integerClass, "<init>", "(I)V");
		jobject obj = g_env->NewObject(integerClass, constructor, (jint)99);
		data[0].cdt_val.handle_val->handle = g_env->NewGlobalRef(obj);
		data[0].cdt_val.handle_val->runtime_id = JVM_RUNTIME_ID;
		data[0].free_required = true;

		// Deserialize
		cdts_jvm_serializer deser(g_env, data);
		jobject extracted = deser.extract_handle();

		CHECK(extracted != nullptr);
		jclass cls = g_env->GetObjectClass(extracted);
		jmethodID intValue = g_env->GetMethodID(cls, "intValue", "()I");
		jint value = g_env->CallIntMethod(extracted, intValue);
		CHECK(value == 99);

		g_env->DeleteLocalRef(cls);
		g_env->DeleteLocalRef(obj);
		g_env->DeleteLocalRef(integerClass);
	}

	TEST_CASE("Deserialize pre-filled CDTS with callable")
	{
		// Manually create callable
		metaffi_type param_types[] = {metaffi_int32_type, metaffi_bool_type};
		metaffi_type retval_types[] = {metaffi_string8_type};

		metaffi_type* param_types_ptr = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 2);
		metaffi_type* retval_types_ptr = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 1);
		memcpy(param_types_ptr, param_types, sizeof(metaffi_type) * 2);
		memcpy(retval_types_ptr, retval_types, sizeof(metaffi_type) * 1);

		cdts data(1);
		data[0].type = metaffi_callable_type;
		data[0].cdt_val.callable_val = new cdt_metaffi_callable();
		data[0].cdt_val.callable_val->val = (void*)0x5678EFAB;
		data[0].cdt_val.callable_val->parameters_types = param_types_ptr;
		data[0].cdt_val.callable_val->params_types_length = 2;
		data[0].cdt_val.callable_val->retval_types = retval_types_ptr;
		data[0].cdt_val.callable_val->retval_types_length = 1;
		data[0].free_required = true;

		// Deserialize
		cdts_jvm_serializer deser(g_env, data);
		jobject extracted = deser.extract_handle(); // extract_handle handles callables

		// Verify callable data
		CHECK(data[0].cdt_val.callable_val->val == (void*)0x5678EFAB);
		CHECK(data[0].cdt_val.callable_val->params_types_length == 2);
		CHECK(data[0].cdt_val.callable_val->retval_types_length == 1);
		CHECK(data[0].cdt_val.callable_val->parameters_types[0] == metaffi_int32_type);
		CHECK(data[0].cdt_val.callable_val->parameters_types[1] == metaffi_bool_type);
		CHECK(data[0].cdt_val.callable_val->retval_types[0] == metaffi_string8_type);
	}

	TEST_CASE("Type query on pre-filled CDTS before deserialization")
	{
		// Manually create CDTS with int32, string, float64, null
		cdts data(4);
		data[0] = (metaffi_int32)42;
		data[1].set_string((const char8_t*)"test", true);
		data[1].type = metaffi_string8_type;
		data[2] = (metaffi_float64)3.14;
		data[3].type = metaffi_null_type;

		cdts_jvm_serializer deser(g_env, data);

		// Peek types without consuming
		CHECK(deser.peek_type() == metaffi_int32_type);
		jint i = deser.extract_int();
		CHECK(i == 42);

		CHECK(deser.peek_type() == metaffi_string8_type);
		jstring s = deser.extract_string();
		const char* str_chars = g_env->GetStringUTFChars(s, nullptr);
		CHECK(std::string(str_chars) == "test");
		g_env->ReleaseStringUTFChars(s, str_chars);

		CHECK(deser.peek_type() == metaffi_float64_type);
		jdouble d = deser.extract_double();
		CHECK(d == doctest::Approx(3.14));

		CHECK(deser.peek_type() == metaffi_null_type);
		CHECK(deser.is_null() == true);
	}

	// ===== Packed Array Tests =====

	TEST_CASE("Packed int32 array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create Java int array
		jintArray arr = g_env->NewIntArray(5);
		jint values[] = {10, 20, 30, 40, 50};
		g_env->SetIntArrayRegion(arr, 0, 5, values);

		ser.add_packed_array(arr, metaffi_int32_type);

		// Verify CDT type is packed
		CHECK(metaffi_is_packed_array(data[0].type));
		CHECK(metaffi_packed_element_type(data[0].type) == metaffi_int32_type);

		// Extract
		ser.reset();
		jintArray extracted = (jintArray)ser.extract_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 5);
		jint* elements = g_env->GetIntArrayElements(extracted, nullptr);
		for (int i = 0; i < 5; i++)
		{
			CHECK(elements[i] == values[i]);
		}
		g_env->ReleaseIntArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Packed float64 array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jdoubleArray arr = g_env->NewDoubleArray(3);
		jdouble values[] = {1.1, 2.2, 3.3};
		g_env->SetDoubleArrayRegion(arr, 0, 3, values);

		ser.add_packed_array(arr, metaffi_float64_type);

		CHECK(metaffi_is_packed_array(data[0].type));

		ser.reset();
		jdoubleArray extracted = (jdoubleArray)ser.extract_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);
		jdouble* elements = g_env->GetDoubleArrayElements(extracted, nullptr);
		for (int i = 0; i < 3; i++)
		{
			CHECK(elements[i] == doctest::Approx(values[i]));
		}
		g_env->ReleaseDoubleArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Packed uint8 array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Java byte[] represents uint8 packed array (same bits)
		jbyteArray arr = g_env->NewByteArray(4);
		jbyte values[] = {0, 127, -128, -1}; // 0, 127, 128, 255 as unsigned
		g_env->SetByteArrayRegion(arr, 0, 4, values);

		ser.add_packed_array(arr, metaffi_uint8_type);

		CHECK(metaffi_is_packed_array(data[0].type));
		CHECK(metaffi_packed_element_type(data[0].type) == metaffi_uint8_type);

		ser.reset();
		jbyteArray extracted = (jbyteArray)ser.extract_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 4);
		jbyte* elements = g_env->GetByteArrayElements(extracted, nullptr);
		for (int i = 0; i < 4; i++)
		{
			CHECK(elements[i] == values[i]);
		}
		g_env->ReleaseByteArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Packed bool array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jbooleanArray arr = g_env->NewBooleanArray(3);
		jboolean values[] = {JNI_TRUE, JNI_FALSE, JNI_TRUE};
		g_env->SetBooleanArrayRegion(arr, 0, 3, values);

		ser.add_packed_array(arr, metaffi_bool_type);

		CHECK(metaffi_is_packed_array(data[0].type));

		ser.reset();
		jbooleanArray extracted = (jbooleanArray)ser.extract_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);
		jboolean* elements = g_env->GetBooleanArrayElements(extracted, nullptr);
		CHECK(elements[0] == JNI_TRUE);
		CHECK(elements[1] == JNI_FALSE);
		CHECK(elements[2] == JNI_TRUE);
		g_env->ReleaseBooleanArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Packed string8 array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		// Create String[] in Java
		jclass string_class = g_env->FindClass("java/lang/String");
		jobjectArray arr = g_env->NewObjectArray(3, string_class, nullptr);

		jstring s0 = g_env->NewStringUTF("hello");
		jstring s1 = g_env->NewStringUTF("world");
		jstring s2 = g_env->NewStringUTF("packed");
		g_env->SetObjectArrayElement(arr, 0, s0);
		g_env->SetObjectArrayElement(arr, 1, s1);
		g_env->SetObjectArrayElement(arr, 2, s2);

		ser.add_packed_array((jarray)arr, metaffi_string8_type);

		CHECK(metaffi_is_packed_array(data[0].type));
		CHECK(metaffi_packed_element_type(data[0].type) == metaffi_string8_type);

		ser.reset();
		jobjectArray extracted = (jobjectArray)ser.extract_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);

		jstring e0 = (jstring)g_env->GetObjectArrayElement(extracted, 0);
		jstring e1 = (jstring)g_env->GetObjectArrayElement(extracted, 1);
		jstring e2 = (jstring)g_env->GetObjectArrayElement(extracted, 2);

		const char* c0 = g_env->GetStringUTFChars(e0, nullptr);
		const char* c1 = g_env->GetStringUTFChars(e1, nullptr);
		const char* c2 = g_env->GetStringUTFChars(e2, nullptr);

		CHECK(std::string(c0) == "hello");
		CHECK(std::string(c1) == "world");
		CHECK(std::string(c2) == "packed");

		g_env->ReleaseStringUTFChars(e0, c0);
		g_env->ReleaseStringUTFChars(e1, c1);
		g_env->ReleaseStringUTFChars(e2, c2);

		g_env->DeleteLocalRef(e0);
		g_env->DeleteLocalRef(e1);
		g_env->DeleteLocalRef(e2);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(s0);
		g_env->DeleteLocalRef(s1);
		g_env->DeleteLocalRef(s2);
		g_env->DeleteLocalRef(arr);
		g_env->DeleteLocalRef(string_class);
	}

	TEST_CASE("Packed empty array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jintArray arr = g_env->NewIntArray(0);
		ser.add_packed_array(arr, metaffi_int32_type);

		CHECK(metaffi_is_packed_array(data[0].type));
		cdt_packed_array* packed = data[0].get_packed_array();
		CHECK(packed->length == 0);

		ser.reset();
		jintArray extracted = (jintArray)ser.extract_array();
		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 0);

		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Packed int64 array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jlongArray arr = g_env->NewLongArray(3);
		jlong values[] = {INT64_MIN, 0, INT64_MAX};
		g_env->SetLongArrayRegion(arr, 0, 3, values);

		ser.add_packed_array(arr, metaffi_int64_type);

		ser.reset();
		jlongArray extracted = (jlongArray)ser.extract_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);
		jlong* elements = g_env->GetLongArrayElements(extracted, nullptr);
		CHECK(elements[0] == INT64_MIN);
		CHECK(elements[1] == 0);
		CHECK(elements[2] == INT64_MAX);
		g_env->ReleaseLongArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("Packed float32 array round-trip")
	{
		cdts data(1);
		cdts_jvm_serializer ser(g_env, data);

		jfloatArray arr = g_env->NewFloatArray(3);
		jfloat values[] = {-1.5f, 0.0f, 3.14f};
		g_env->SetFloatArrayRegion(arr, 0, 3, values);

		ser.add_packed_array(arr, metaffi_float32_type);

		ser.reset();
		jfloatArray extracted = (jfloatArray)ser.extract_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);
		jfloat* elements = g_env->GetFloatArrayElements(extracted, nullptr);
		for (int i = 0; i < 3; i++)
		{
			CHECK(elements[i] == doctest::Approx(values[i]));
		}
		g_env->ReleaseFloatArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(extracted);
		g_env->DeleteLocalRef(arr);
	}

	TEST_CASE("extract_packed_array directly")
	{
		// Manually construct a packed CDT and extract via extract_packed_array
		cdts data(1);

		// Build packed int32 array manually
		cdt_packed_array* packed = new cdt_packed_array();
		packed->length = 3;
		int32_t* buf = static_cast<int32_t*>(malloc(3 * sizeof(int32_t)));
		buf[0] = 100; buf[1] = 200; buf[2] = 300;
		packed->data = buf;
		data[0].set_packed_array(packed, metaffi_int32_type);

		cdts_jvm_serializer deser(g_env, data);
		jintArray extracted = (jintArray)deser.extract_packed_array();

		REQUIRE(extracted != nullptr);
		CHECK(g_env->GetArrayLength(extracted) == 3);
		jint* elements = g_env->GetIntArrayElements(extracted, nullptr);
		CHECK(elements[0] == 100);
		CHECK(elements[1] == 200);
		CHECK(elements[2] == 300);
		g_env->ReleaseIntArrayElements(extracted, elements, JNI_ABORT);
		g_env->DeleteLocalRef(extracted);
	}
} // TEST_SUITE
