#pragma once

#include "function_definition.hpp"
#include "class_definition.hpp"
#include "global_definition.hpp"

namespace metaffi::idl {

/**
 * ModuleDefinition represents a module in the IDL
 * Contains functions, classes, globals, external resources, and metadata
 */
class ModuleDefinition {
protected:
	std::string name_;
	std::string comment_;
	std::map<std::string, std::string> tags_;
	std::vector<FunctionDefinition> functions_;
	std::vector<ClassDefinition> classes_;
	std::vector<GlobalDefinition> globals_;
	std::vector<std::string> external_resources_;     // Library names, file paths, etc.

public:
	// Constructors
	ModuleDefinition() = default;

	explicit ModuleDefinition(std::string name)
		: name_(std::move(name))
	{}

	// Rule of 5 (default implementations are fine due to RAII members)
	~ModuleDefinition() = default;
	ModuleDefinition(const ModuleDefinition&) = default;
	ModuleDefinition(ModuleDefinition&&) noexcept = default;
	ModuleDefinition& operator=(const ModuleDefinition&) = default;
	ModuleDefinition& operator=(ModuleDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const std::string& name() const noexcept { return name_; }
	[[nodiscard]] const std::string& comment() const noexcept { return comment_; }
	[[nodiscard]] const std::map<std::string, std::string>& tags() const noexcept { return tags_; }
	[[nodiscard]] const std::vector<FunctionDefinition>& functions() const noexcept { return functions_; }
	[[nodiscard]] const std::vector<ClassDefinition>& classes() const noexcept { return classes_; }
	[[nodiscard]] const std::vector<GlobalDefinition>& globals() const noexcept { return globals_; }
	[[nodiscard]] const std::vector<std::string>& external_resources() const noexcept { return external_resources_; }

	// Mutable getters (for finalize_construction to expand env vars)
	[[nodiscard]] std::vector<std::string>& external_resources_mutable() { return external_resources_; }

	// Setters (fluent API)
	ModuleDefinition& set_name(const std::string& name) { name_ = name; return *this; }
	ModuleDefinition& set_comment(const std::string& comment) { comment_ = comment; return *this; }

	// Tag management
	ModuleDefinition& set_tag(const std::string& key, const std::string& value) {
		tags_[key] = value;
		return *this;
	}

	[[nodiscard]] std::string get_tag(const std::string& key) const {
		auto it = tags_.find(key);
		return it != tags_.end() ? it->second : "";
	}

	// Comment management
	ModuleDefinition& append_comment(const std::string& additional_comment) {
		if (!comment_.empty() && !additional_comment.empty()) {
			comment_ += "\n";
		}
		comment_ += additional_comment;
		return *this;
	}

	// Entity management (builder pattern)
	ModuleDefinition& add_function(const FunctionDefinition& func) {
		functions_.push_back(func);
		return *this;
	}

	ModuleDefinition& add_function(FunctionDefinition&& func) {
		functions_.push_back(std::move(func));
		return *this;
	}

	ModuleDefinition& add_class(const ClassDefinition& cls) {
		classes_.push_back(cls);
		return *this;
	}

	ModuleDefinition& add_class(ClassDefinition&& cls) {
		classes_.push_back(std::move(cls));
		return *this;
	}

	ModuleDefinition& add_global(const GlobalDefinition& global) {
		globals_.push_back(global);
		return *this;
	}

	ModuleDefinition& add_global(GlobalDefinition&& global) {
		globals_.push_back(std::move(global));
		return *this;
	}

	ModuleDefinition& add_external_resource(const std::string& resource) {
		external_resources_.push_back(resource);
		return *this;
	}

	// JSON serialization
	[[nodiscard]] nlohmann::json to_json() const {
		nlohmann::json j;
		j["name"] = name_;
		j["comment"] = comment_;
		j["tags"] = tags_;
		j["functions"] = nlohmann::json::array();
		j["classes"] = nlohmann::json::array();
		j["globals"] = nlohmann::json::array();
		j["external_resources"] = nlohmann::json::array();

		// Serialize functions
		for (const auto& func : functions_) {
			j["functions"].push_back(func.to_json());
		}

		// Serialize classes
		for (const auto& cls : classes_) {
			j["classes"].push_back(cls.to_json());
		}

		// Serialize globals
		for (const auto& global : globals_) {
			j["globals"].push_back(global.to_json());
		}

		// Serialize external resources
		for (const auto& resource : external_resources_) {
			j["external_resources"].push_back(resource);
		}

		return j;
	}

	// JSON deserialization
	static ModuleDefinition from_json(const nlohmann::json& j) {
		ModuleDefinition module;

		// Required fields with defaults
		module.name_ = j.value("name", "");
		module.comment_ = j.value("comment", "");

		// Tags (optional)
		if (j.contains("tags") && j["tags"].is_object()) {
			module.tags_ = j["tags"].get<std::map<std::string, std::string>>();
		}

		// Functions (optional)
		if (j.contains("functions") && j["functions"].is_array()) {
			for (const auto& func_json : j["functions"]) {
				module.functions_.push_back(FunctionDefinition::from_json(func_json));
			}
		}

		// Classes (optional)
		if (j.contains("classes") && j["classes"].is_array()) {
			for (const auto& class_json : j["classes"]) {
				module.classes_.push_back(ClassDefinition::from_json(class_json));
			}
		}

		// Globals (optional)
		if (j.contains("globals") && j["globals"].is_array()) {
			for (const auto& global_json : j["globals"]) {
				module.globals_.push_back(GlobalDefinition::from_json(global_json));
			}
		}

		// External resources (optional)
		if (j.contains("external_resources") && j["external_resources"].is_array()) {
			for (const auto& resource : j["external_resources"]) {
				if (resource.is_string()) {
					module.external_resources_.push_back(resource.get<std::string>());
				}
			}
		}

		// Validation: fail-fast on invalid data
		if (module.name_.empty()) {
			throw IDLException("MISSING_REQUIRED_FIELD", "ModuleDefinition requires 'name' field");
		}

		return module;
	}

	// Equality comparison
	bool operator==(const ModuleDefinition& other) const {
		return name_ == other.name_ &&
		       comment_ == other.comment_ &&
		       tags_ == other.tags_ &&
		       functions_ == other.functions_ &&
		       classes_ == other.classes_ &&
		       globals_ == other.globals_ &&
		       external_resources_ == other.external_resources_;
	}

	bool operator!=(const ModuleDefinition& other) const {
		return !(*this == other);
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const ModuleDefinition& module) {
	j = module.to_json();
}

inline void from_json(const nlohmann::json& j, ModuleDefinition& module) {
	module = ModuleDefinition::from_json(j);
}

} // namespace metaffi::idl
