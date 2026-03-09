#pragma once

#include <string>
#include <vector>
#include <cstddef>


// ============================================================================
// Info structs — mirrors the Python3 extractor's dataclasses
// ============================================================================

struct CppParamInfo
{
	std::string name;
	std::string type_str;
	bool        is_optional = false;
};

struct CppFunctionInfo
{
	std::string name;
	std::string mangled_name;      // from clang_Cursor_getMangling()
	std::string ns;                // "outer::inner" or "" for global namespace
	std::vector<CppParamInfo> params;
	std::vector<std::string>  return_types;
	bool is_static = false;        // true for class static methods
};

struct CppFieldInfo
{
	std::string name;
	std::string type_str;
	long long   offset_bytes = 0;  // byte offset within the class
	long long   size_bytes   = 0;  // sizeof(field)
};

struct CppClassInfo
{
	std::string name;
	std::string ns;                // namespace path or ""
	std::string mangled_ctor;      // mangled constructor symbol
	std::string mangled_dtor;      // mangled destructor symbol
	std::size_t class_size   = 0; // sizeof(class)
	std::vector<CppFunctionInfo> constructors;
	std::vector<CppFunctionInfo> methods;
	std::vector<CppFieldInfo>    fields;
	std::vector<CppFunctionInfo> static_methods;
	bool has_destructor = false;
};

struct CppGlobalInfo
{
	std::string name;
	std::string mangled_name;
	std::string ns;
	std::string type_str;
};

struct CppModuleInfo
{
	std::string header_path;
	std::vector<CppFunctionInfo> functions; // top-level & namespaced free functions
	std::vector<CppGlobalInfo>   globals;
	std::vector<CppClassInfo>    classes;
};


// ============================================================================
// CppExtractor — uses libclang to parse a C/C++ header file
// ============================================================================

/**
 * CppExtractor
 *
 * Parses a C/C++ header file using libclang and returns a CppModuleInfo
 * containing all top-level and namespaced free functions, global variables,
 * and classes with their members.
 *
 * Usage:
 *   CppExtractor ex("path/to/header.h");
 *   CppModuleInfo info = ex.extract();
 */
class CppExtractor
{
public:
	/**
	 * @param header_path  Absolute or relative path to the .h file to parse.
	 * @throws std::runtime_error if the file cannot be parsed.
	 */
	explicit CppExtractor(const std::string& header_path);
	~CppExtractor();

	// Non-copyable (owns libclang resources)
	CppExtractor(const CppExtractor&)            = delete;
	CppExtractor& operator=(const CppExtractor&) = delete;

	/**
	 * Run the extraction and return the module info.
	 * @throws std::runtime_error on parse error.
	 */
	CppModuleInfo extract();

private:
	struct Impl;
	Impl* m_impl = nullptr;
};
