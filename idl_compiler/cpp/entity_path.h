#pragma once

#include <map>
#include <string>
#include <cstddef>


// Forward declarations of info structs (defined in extractor.h)
struct CppFunctionInfo;
struct CppGlobalInfo;
struct CppClassInfo;
struct CppFieldInfo;


/**
 * CppEntityPathGenerator
 *
 * Produces entity_path maps (std::map<std::string,std::string>) from
 * extracted C++ interface info. These maps are serialised to comma-separated
 * key=value pairs by the IDL generator.
 *
 * All optional namespace= keys are omitted when the namespace string is empty.
 */
class CppEntityPathGenerator
{
public:
	// All methods are static — no instances needed.
	CppEntityPathGenerator() = delete;

	// -----------------------------------------------------------------
	// Free function / static method
	// -----------------------------------------------------------------

	/**
	 * Entity path for a C++ free function or static method.
	 * Entity path: callable=<mangled_name>[,namespace=<ns>]
	 */
	static std::map<std::string, std::string> free_function(const CppFunctionInfo& fn);

	// -----------------------------------------------------------------
	// Instance method
	// -----------------------------------------------------------------

	/**
	 * Entity path for a C++ instance method.
	 * Entity path: callable=<mangled_name>,instance_required=true[,namespace=<ns>]
	 */
	static std::map<std::string, std::string> instance_method(const CppFunctionInfo& fn);

	// -----------------------------------------------------------------
	// Constructor
	// -----------------------------------------------------------------

	/**
	 * Entity path for a C++ constructor.
	 * Entity path: callable=<mangled_ctor>,constructor=true,class_size=<N>[,namespace=<ns>]
	 *
	 * @param mangled_ctor  Mangled constructor symbol name.
	 * @param class_size    sizeof(class) — number of bytes to allocate.
	 * @param ns            Namespace string (may be empty).
	 */
	static std::map<std::string, std::string> constructor(
		const std::string& mangled_ctor,
		std::size_t        class_size,
		const std::string& ns = "");

	// -----------------------------------------------------------------
	// Destructor
	// -----------------------------------------------------------------

	/**
	 * Entity path for a C++ destructor.
	 * Entity path: callable=<mangled_dtor>,destructor=true[,namespace=<ns>]
	 *
	 * @param mangled_dtor  Mangled destructor symbol name.
	 * @param ns            Namespace string (may be empty).
	 */
	static std::map<std::string, std::string> destructor(
		const std::string& mangled_dtor,
		const std::string& ns = "");

	// -----------------------------------------------------------------
	// Global variable getter / setter
	// -----------------------------------------------------------------

	/**
	 * Entity path for a C++ global variable getter.
	 * Entity path: global=<mangled_name>,getter=true[,namespace=<ns>]
	 */
	static std::map<std::string, std::string> global_getter(const CppGlobalInfo& g);

	/**
	 * Entity path for a C++ global variable setter.
	 * Entity path: global=<mangled_name>,setter=true[,namespace=<ns>]
	 */
	static std::map<std::string, std::string> global_setter(const CppGlobalInfo& g);

	// -----------------------------------------------------------------
	// Field getter / setter
	// -----------------------------------------------------------------

	/**
	 * Entity path for a C++ field getter.
	 * Entity path: field=<name>,getter=true,instance_required=true,
	 *              field_offset=<offset>[,namespace=<ns>]
	 *
	 * @param field       Field info (name + offset_bytes).
	 * @param ns          Namespace / class namespace string.
	 */
	static std::map<std::string, std::string> field_getter(
		const CppFieldInfo& field,
		const std::string&  ns = "");

	/**
	 * Entity path for a C++ field setter.
	 * Entity path: field=<name>,setter=true,instance_required=true,
	 *              field_offset=<offset>,field_size=<size>[,namespace=<ns>]
	 */
	static std::map<std::string, std::string> field_setter(
		const CppFieldInfo& field,
		const std::string&  ns = "");

	// -----------------------------------------------------------------
	// Serialisation helper
	// -----------------------------------------------------------------

	/**
	 * Serialise an entity_path map to "key=value,key=value" string.
	 * Entries with empty values are skipped.
	 */
	static std::string serialise(const std::map<std::string, std::string>& path_map);
};
