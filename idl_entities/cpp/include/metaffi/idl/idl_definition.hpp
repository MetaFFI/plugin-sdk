#pragma once

#include "module_definition.hpp"
#include <fstream>
#include <regex>
#include <filesystem>
#include <utils/safe_func.h>

namespace metaffi::idl {

/**
 * IDLDefinition represents the root of an IDL file
 * Contains metadata and modules
 */
class IDLDefinition {
protected:
	std::string idl_source_;                      // Filename without extension
	std::string idl_extension_;                   // File extension (e.g., '.json', '.idl')
	std::string idl_filename_with_extension_;     // Full filename with extension
	std::string idl_full_path_;                   // Absolute path to IDL file
	std::string metaffi_guest_lib_;               // Name of guest library (auto-generated from idl_source)
	std::string target_language_;                 // Target language for compilation (e.g., 'cpp', 'python3')
	std::vector<ModuleDefinition> modules_;

public:
	// Constructors
	IDLDefinition() = default;

	IDLDefinition(std::string idl_source, std::string target_language)
		: idl_source_(std::move(idl_source))
		, target_language_(std::move(target_language))
	{
		// Auto-generate metaffi_guest_lib from idl_source
		metaffi_guest_lib_ = "guest_" + idl_source_;
	}

	// Rule of 5 (default implementations are fine due to RAII members)
	~IDLDefinition() = default;
	IDLDefinition(const IDLDefinition&) = default;
	IDLDefinition(IDLDefinition&&) noexcept = default;
	IDLDefinition& operator=(const IDLDefinition&) = default;
	IDLDefinition& operator=(IDLDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const std::string& idl_source() const noexcept { return idl_source_; }
	[[nodiscard]] const std::string& idl_extension() const noexcept { return idl_extension_; }
	[[nodiscard]] const std::string& idl_filename_with_extension() const noexcept { return idl_filename_with_extension_; }
	[[nodiscard]] const std::string& idl_full_path() const noexcept { return idl_full_path_; }
	[[nodiscard]] const std::string& metaffi_guest_lib() const noexcept { return metaffi_guest_lib_; }
	[[nodiscard]] const std::string& target_language() const noexcept { return target_language_; }
	[[nodiscard]] const std::vector<ModuleDefinition>& modules() const noexcept { return modules_; }

	// Setters (fluent API)
	IDLDefinition& set_idl_source(const std::string& source) {
		idl_source_ = source;
		// Auto-generate metaffi_guest_lib
		metaffi_guest_lib_ = "guest_" + idl_source_;
		return *this;
	}

	IDLDefinition& set_idl_extension(const std::string& ext) { idl_extension_ = ext; return *this; }
	IDLDefinition& set_idl_filename_with_extension(const std::string& filename) { idl_filename_with_extension_ = filename; return *this; }
	IDLDefinition& set_idl_full_path(const std::string& path) { idl_full_path_ = path; return *this; }
	IDLDefinition& set_metaffi_guest_lib(const std::string& lib) { metaffi_guest_lib_ = lib; return *this; }
	IDLDefinition& set_target_language(const std::string& lang) { target_language_ = lang; return *this; }

	// Module management (builder pattern)
	IDLDefinition& add_module(const ModuleDefinition& module) {
		modules_.push_back(module);
		return *this;
	}

	IDLDefinition& add_module(ModuleDefinition&& module) {
		modules_.push_back(std::move(module));
		return *this;
	}

	// Environment variable expansion
	// Expands ${VAR} or %VAR% patterns in external_resources
	void finalize_construction() {
		for (auto& module : modules_) {
			auto& resources = module.external_resources_mutable();
			for (auto& resource : resources) {
				resource = expand_env_vars(resource);
			}
		}
	}

	// File I/O with RAII
	void save_to_file(const std::string& path) const {
		std::ofstream file(path);
		if (!file) {
			throw IDLException("FILE_WRITE_ERROR", "Failed to open file for writing: " + path);
		}
		file << to_json().dump(2);  // Pretty print with 2-space indent
		// RAII: file auto-closes on scope exit
	}

	// Load from file
	static IDLDefinition load_from_file(const std::string& path) {
		std::ifstream file(path);
		if (!file) {
			throw IDLException("FILE_READ_ERROR", "Failed to open file for reading: " + path);
		}

		nlohmann::json j;
		try {
			file >> j;  // Parse JSON
		} catch (const nlohmann::json::exception& e) {
			throw IDLException("JSON_PARSE_ERROR", std::string("Failed to parse JSON: ") + e.what(), "File: " + path);
		}

		// RAII: file auto-closes
		auto idl = from_json(j);

		// Set file metadata if not already set
		if (idl.idl_full_path_.empty()) {
			idl.idl_full_path_ = std::filesystem::absolute(path).string();
		}
		if (idl.idl_filename_with_extension_.empty()) {
			idl.idl_filename_with_extension_ = std::filesystem::path(path).filename().string();
		}

		return idl;
	}

	// Load from JSON string
	static IDLDefinition load_from_json(const std::string& json_str) {
		try {
			auto j = nlohmann::json::parse(json_str);
			return from_json(j);
		} catch (const nlohmann::json::exception& e) {
			throw IDLException("JSON_PARSE_ERROR", std::string("Failed to parse JSON string: ") + e.what());
		}
	}

	// JSON serialization
	[[nodiscard]] nlohmann::json to_json() const {
		nlohmann::json j;
		j["idl_source"] = idl_source_;
		j["idl_extension"] = idl_extension_;
		j["idl_filename_with_extension"] = idl_filename_with_extension_;
		j["idl_full_path"] = idl_full_path_;
		j["metaffi_guest_lib"] = metaffi_guest_lib_;
		j["target_language"] = target_language_;
		j["modules"] = nlohmann::json::array();

		// Serialize modules
		for (const auto& module : modules_) {
			j["modules"].push_back(module.to_json());
		}

		return j;
	}

	// JSON deserialization
	static IDLDefinition from_json(const nlohmann::json& j) {
		IDLDefinition idl;

		// Required fields with defaults
		idl.idl_source_ = j.value("idl_source", "");
		idl.idl_extension_ = j.value("idl_extension", "");
		idl.idl_filename_with_extension_ = j.value("idl_filename_with_extension", "");
		idl.idl_full_path_ = j.value("idl_full_path", "");
		idl.metaffi_guest_lib_ = j.value("metaffi_guest_lib", "");
		idl.target_language_ = j.value("target_language", "");

		// Modules (optional)
		if (j.contains("modules") && j["modules"].is_array()) {
			for (const auto& module_json : j["modules"]) {
				idl.modules_.push_back(ModuleDefinition::from_json(module_json));
			}
		}

		return idl;
	}

	// Equality comparison
	bool operator==(const IDLDefinition& other) const {
		return idl_source_ == other.idl_source_ &&
		       idl_extension_ == other.idl_extension_ &&
		       idl_filename_with_extension_ == other.idl_filename_with_extension_ &&
		       idl_full_path_ == other.idl_full_path_ &&
		       metaffi_guest_lib_ == other.metaffi_guest_lib_ &&
		       target_language_ == other.target_language_ &&
		       modules_ == other.modules_;
	}

	bool operator!=(const IDLDefinition& other) const {
		return !(*this == other);
	}

private:
	// Helper: Expand environment variables in a string
	// Supports ${VAR} and %VAR% patterns
	static std::string expand_env_vars(const std::string& input) {
		std::string result = input;

		// Expand ${VAR} pattern (Unix-style)
		std::regex unix_pattern(R"(\$\{([^}]+)\})");
		std::smatch match;
		while (std::regex_search(result, match, unix_pattern)) {
			std::string var_name = match[1].str();
			char* env_value = metaffi_getenv_alloc(var_name.c_str());  // SAFE - allocates on Windows
			std::string replacement = env_value ? env_value : "";
			metaffi_free_env(env_value);  // SAFE - frees on Windows, no-op on Unix
			result.replace(match.position(), match.length(), replacement);
		}

		// Expand %VAR% pattern (Windows-style)
		std::regex windows_pattern(R"(%([^%]+)%)");
		while (std::regex_search(result, match, windows_pattern)) {
			std::string var_name = match[1].str();
			char* env_value = metaffi_getenv_alloc(var_name.c_str());  // SAFE - allocates on Windows
			std::string replacement = env_value ? env_value : "";
			metaffi_free_env(env_value);  // SAFE - frees on Windows, no-op on Unix
			result.replace(match.position(), match.length(), replacement);
		}

		return result;
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const IDLDefinition& idl) {
	j = idl.to_json();
}

inline void from_json(const nlohmann::json& j, IDLDefinition& idl) {
	idl = IDLDefinition::from_json(j);
}

// Implementation of entity_path_as_string for FunctionDefinition
// (Defined here because it needs IDLDefinition)
inline std::string FunctionDefinition::entity_path_as_string(const IDLDefinition& idl) const {
	// Collect all keys (entity_path + implicit metaffi_guest_lib)
	std::set<std::string> keys;
	for (const auto& [key, _] : entity_path_) {
		keys.insert(key);
	}
	keys.insert("metaffi_guest_lib");

	// Build sorted "key=value" string
	std::ostringstream result;
	bool first = true;

	for (const auto& key : keys) {
		std::string value;

		if (auto it = entity_path_.find(key); it != entity_path_.end()) {
			value = it->second;
		} else if (key == "metaffi_guest_lib") {
			value = idl.metaffi_guest_lib();
		} else {
			throw IDLException("MISSING_ENTITY_PATH_KEY", "Unexpected key in entity path",
			                   "Function: " + name_ + ", Key: " + key);
		}

		if (!first) result << ",";
		result << key << "=" << value;
		first = false;
	}

	return result.str();
}

// Implementation of entity_path_as_string for MethodDefinition
// (Merges parent class entity_path with method entity_path)
inline std::string MethodDefinition::entity_path_as_string(const IDLDefinition& idl) const {
	// Collect all keys from both parent class and method entity_path
	std::set<std::string> keys;

	// Add parent class entity_path keys if parent exists
	if (parent_class_) {
		for (const auto& [key, _] : parent_class_->entity_path()) {
			keys.insert(key);
		}
	}

	// Add method entity_path keys (overrides parent)
	for (const auto& [key, _] : entity_path_) {
		keys.insert(key);
	}

	keys.insert("metaffi_guest_lib");

	// Build sorted "key=value" string
	std::ostringstream result;
	bool first = true;

	for (const auto& key : keys) {
		std::string value;

		// Try method entity_path first (overrides parent)
		if (auto it = entity_path_.find(key); it != entity_path_.end()) {
			value = it->second;
		}
		// Then try parent class entity_path
		else if (parent_class_) {
			if (auto it = parent_class_->entity_path().find(key); it != parent_class_->entity_path().end()) {
				value = it->second;
			}
		}
		// Finally, implicit metaffi_guest_lib
		if (value.empty() && key == "metaffi_guest_lib") {
			value = idl.metaffi_guest_lib();
		}

		if (value.empty()) {
			throw IDLException("MISSING_ENTITY_PATH_KEY", "Unexpected key in entity path",
			                   "Method: " + name_ + ", Key: " + key);
		}

		if (!first) result << ",";
		result << key << "=" << value;
		first = false;
	}

	return result.str();
}

} // namespace metaffi::idl
