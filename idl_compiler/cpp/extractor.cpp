#include "extractor.h"

#include <clang-c/Index.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


// ============================================================================
// libclang string helper (RAII)
// ============================================================================

namespace
{
	// Converts a CXString to std::string and disposes the CXString.
	std::string cx_to_string(CXString cx)
	{
		const char* cstr = clang_getCString(cx);
		std::string result = cstr ? cstr : "";
		clang_disposeString(cx);
		return result;
	}

	// Build namespace path from a cursor by walking parent cursors.
	std::string get_namespace_path(CXCursor cursor)
	{
		std::vector<std::string> parts;

		CXCursor parent = clang_getCursorSemanticParent(cursor);
		while (!clang_Cursor_isNull(parent))
		{
			CXCursorKind kind = clang_getCursorKind(parent);
			if (kind == CXCursor_Namespace)
			{
				parts.push_back(cx_to_string(clang_getCursorSpelling(parent)));
			}
			else if (kind == CXCursor_TranslationUnit)
			{
				break;
			}
			else
			{
				// Class or other scope — stop namespace aggregation
				break;
			}
			parent = clang_getCursorSemanticParent(parent);
		}

		// Reverse because we walked bottom-up
		std::reverse(parts.begin(), parts.end());

		std::string ns;
		for (std::size_t i = 0; i < parts.size(); ++i)
		{
			if (i > 0) ns += "::";
			ns += parts[i];
		}
		return ns;
	}

	// Extract the mangled symbol name for a cursor.
	std::string get_mangled_name(CXCursor cursor)
	{
		return cx_to_string(clang_Cursor_getMangling(cursor));
	}

	// Convert CXType spelling to a normalised C++ type string.
	std::string type_spelling(CXType t)
	{
		return cx_to_string(clang_getTypeSpelling(t));
	}

	// Extract parameter info from a function/method cursor.
	std::vector<CppParamInfo> extract_params(CXCursor func_cursor)
	{
		std::vector<CppParamInfo> params;
		int num_args = clang_Cursor_getNumArguments(func_cursor);

		for (int i = 0; i < num_args; ++i)
		{
			CXCursor arg = clang_Cursor_getArgument(func_cursor, i);
			CppParamInfo p;
			p.name     = cx_to_string(clang_getCursorSpelling(arg));
			p.type_str = type_spelling(clang_getCursorType(arg));
			params.push_back(std::move(p));
		}

		return params;
	}

	// Extract return type string from a function cursor.
	std::string extract_return_type(CXCursor func_cursor)
	{
		CXType func_type   = clang_getCursorType(func_cursor);
		CXType result_type = clang_getResultType(func_type);
		return type_spelling(result_type);
	}

} // anonymous namespace


// ============================================================================
// Visitor data
// ============================================================================

struct VisitorData
{
	CppModuleInfo* info = nullptr;
};

// Forward declarations
static CXChildVisitResult visit_toplevel(CXCursor cursor, CXCursor parent, CXClientData data);
static CXChildVisitResult visit_class(CXCursor cursor, CXCursor parent, CXClientData data);


// ============================================================================
// Class visitor
// ============================================================================

struct ClassVisitorData
{
	CppClassInfo* class_info = nullptr;
};

static CXChildVisitResult visit_class(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
	auto* vd        = static_cast<ClassVisitorData*>(data);
	CppClassInfo* ci = vd->class_info;

	CXCursorKind kind = clang_getCursorKind(cursor);

	// Only consider public members
	CX_CXXAccessSpecifier access = clang_getCXXAccessSpecifier(cursor);
	if (access != CX_CXXPublic) return CXChildVisit_Continue;

	if (kind == CXCursor_Constructor)
	{
		CppFunctionInfo fn;
		fn.name         = cx_to_string(clang_getCursorSpelling(cursor));
		fn.mangled_name = get_mangled_name(cursor);
		fn.ns           = ci->ns;
		fn.params       = extract_params(cursor);
		fn.return_types = {ci->name}; // constructor returns the class type
		fn.is_static    = false;

		// Store the first constructor's mangled name as the class ctor
		if (ci->mangled_ctor.empty())
		{
			ci->mangled_ctor = fn.mangled_name;
		}

		ci->constructors.push_back(std::move(fn));
	}
	else if (kind == CXCursor_Destructor)
	{
		ci->has_destructor  = true;
		ci->mangled_dtor    = get_mangled_name(cursor);
	}
	else if (kind == CXCursor_CXXMethod)
	{
		CppFunctionInfo fn;
		fn.name         = cx_to_string(clang_getCursorSpelling(cursor));
		fn.mangled_name = get_mangled_name(cursor);
		fn.ns           = ci->ns;
		fn.params       = extract_params(cursor);
		fn.is_static    = clang_CXXMethod_isStatic(cursor) != 0;

		std::string ret = extract_return_type(cursor);
		if (ret != "void") fn.return_types = {ret};

		if (fn.is_static)
		{
			ci->static_methods.push_back(std::move(fn));
		}
		else
		{
			ci->methods.push_back(std::move(fn));
		}
	}
	else if (kind == CXCursor_FieldDecl)
	{
		CppFieldInfo field;
		field.name = cx_to_string(clang_getCursorSpelling(cursor));
		field.type_str = type_spelling(clang_getCursorType(cursor));

		// Field offset in bytes
		long long offset_bits = clang_Cursor_getOffsetOfField(cursor);
		field.offset_bytes = (offset_bits >= 0) ? (offset_bits / 8) : 0;

		// Field size in bytes via type width
		CXType field_type  = clang_getCursorType(cursor);
		long long size_bits = clang_Type_getSizeOf(field_type);
		field.size_bytes = (size_bits > 0) ? size_bits : 0;

		ci->fields.push_back(std::move(field));
	}

	return CXChildVisit_Continue;
}


// ============================================================================
// Top-level visitor
// ============================================================================

static CXChildVisitResult visit_toplevel(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
	auto* vd = static_cast<VisitorData*>(data);

	// Skip cursors not from the main file (i.e., from system includes)
	CXSourceLocation loc = clang_getCursorLocation(cursor);
	if (clang_Location_isInSystemHeader(loc)) return CXChildVisit_Continue;

	CXCursorKind kind = clang_getCursorKind(cursor);

	if (kind == CXCursor_Namespace)
	{
		// Recurse into namespace — the top-level visitor will be called again
		// for each child, and the namespace path is re-derived per entity.
		clang_visitChildren(cursor, visit_toplevel, data);
	}
	else if (kind == CXCursor_FunctionDecl || kind == CXCursor_FunctionTemplate)
	{
		CppFunctionInfo fn;
		fn.name         = cx_to_string(clang_getCursorSpelling(cursor));
		fn.mangled_name = get_mangled_name(cursor);
		fn.ns           = get_namespace_path(cursor);
		fn.params       = extract_params(cursor);
		fn.is_static    = false;

		std::string ret = extract_return_type(cursor);
		if (ret != "void") fn.return_types = {ret};

		vd->info->functions.push_back(std::move(fn));
	}
	else if (kind == CXCursor_VarDecl)
	{
		// Global (or namespace-scope) variable
		CppGlobalInfo g;
		g.name         = cx_to_string(clang_getCursorSpelling(cursor));
		g.mangled_name = get_mangled_name(cursor);
		g.ns           = get_namespace_path(cursor);
		g.type_str     = type_spelling(clang_getCursorType(cursor));

		vd->info->globals.push_back(std::move(g));
	}
	else if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl)
	{
		// Only process class definitions (not forward declarations)
		if (!clang_isCursorDefinition(cursor)) return CXChildVisit_Continue;

		CppClassInfo ci;
		ci.name = cx_to_string(clang_getCursorSpelling(cursor));
		ci.ns   = get_namespace_path(cursor);

		// Get sizeof(class) from the type
		CXType class_type = clang_getCursorType(cursor);
		long long size = clang_Type_getSizeOf(class_type);
		ci.class_size = (size > 0) ? static_cast<std::size_t>(size) : 0;

		// Visit class members
		ClassVisitorData cvd{ &ci };
		clang_visitChildren(cursor, visit_class, &cvd);

		vd->info->classes.push_back(std::move(ci));
	}

	return CXChildVisit_Continue;
}


// ============================================================================
// CppExtractor::Impl
// ============================================================================

struct CppExtractor::Impl
{
	std::string header_path;
	CXIndex     index = nullptr;
	CXTranslationUnit tu = nullptr;

	Impl() = default;

	~Impl()
	{
		if (tu)    clang_disposeTranslationUnit(tu);
		if (index) clang_disposeIndex(index);
	}
};


// ============================================================================
// CppExtractor
// ============================================================================

CppExtractor::CppExtractor(const std::string& header_path)
	: m_impl(new Impl())
{
	if (!std::filesystem::exists(header_path))
	{
		delete m_impl;
		m_impl = nullptr;
		throw std::runtime_error("CppExtractor: header file not found: " + header_path);
	}

	m_impl->header_path = header_path;

	m_impl->index = clang_createIndex(
		/*excludeDeclarationsFromPCH=*/0,
		/*displayDiagnostics=*/0);

	if (!m_impl->index)
	{
		delete m_impl;
		m_impl = nullptr;
		throw std::runtime_error("CppExtractor: failed to create libclang index");
	}

	// Parse with C++17 standard; include the directory of the header for local includes.
	const char* args[] = {
		"-std=c++17",
		"-x", "c++-header"
	};

	m_impl->tu = clang_parseTranslationUnit(
		m_impl->index,
		header_path.c_str(),
		args, 3,
		nullptr, 0,
		CXTranslationUnit_DetailedPreprocessingRecord |
		CXTranslationUnit_SkipFunctionBodies);

	if (!m_impl->tu)
	{
		throw std::runtime_error("CppExtractor: failed to parse header: " + header_path);
	}

	// Report any fatal diagnostics
	unsigned num_diag = clang_getNumDiagnostics(m_impl->tu);
	for (unsigned i = 0; i < num_diag; ++i)
	{
		CXDiagnostic diag = clang_getDiagnostic(m_impl->tu, i);
		CXDiagnosticSeverity sev = clang_getDiagnosticSeverity(diag);
		if (sev >= CXDiagnostic_Error)
		{
			std::string msg = cx_to_string(clang_formatDiagnostic(diag, CXDiagnostic_DisplaySourceLocation));
			clang_disposeDiagnostic(diag);
			throw std::runtime_error("CppExtractor: parse error in '" + header_path + "': " + msg);
		}
		clang_disposeDiagnostic(diag);
	}
}

CppExtractor::~CppExtractor()
{
	delete m_impl;
}

CppModuleInfo CppExtractor::extract()
{
	CppModuleInfo info;
	info.header_path = m_impl->header_path;

	CXCursor root = clang_getTranslationUnitCursor(m_impl->tu);

	VisitorData vd{ &info };
	clang_visitChildren(root, visit_toplevel, &vd);

	return info;
}
