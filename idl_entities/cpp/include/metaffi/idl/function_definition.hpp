#pragma once

#include "arg_definition.hpp"
#include <vector>
#include <set>
#include <sstream>

namespace metaffi::idl {

// Forward declaration
class IDLDefinition;

/**
 * FunctionDefinition represents a module-level function in the IDL
 * Contains parameters, return values, entity path routing info, and metadata
 */
class FunctionDefinition {
protected:
	std::string name_;
	std::string comment_;
	std::map<std::string, std::string> tags_;
	std::map<std::string, std::string> entity_path_;
	std::vector<ArgDefinition> parameters_;
	std::vector<ArgDefinition> return_values_;
	int overload_index_ = 0;                       // 0 = not overloaded, 1+ = overload number

public:
	// Constructors
	FunctionDefinition() = default;

	explicit FunctionDefinition(std::string name)
		: name_(std::move(name))
	{}

	// Rule of 5
	virtual ~FunctionDefinition() = default;
	FunctionDefinition(const FunctionDefinition&) = default;
	FunctionDefinition(FunctionDefinition&&) noexcept = default;
	FunctionDefinition& operator=(const FunctionDefinition&) = default;
	FunctionDefinition& operator=(FunctionDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const std::string& name() const noexcept { return name_; }
	[[nodiscard]] const std::string& comment() const noexcept { return comment_; }
	[[nodiscard]] const std::map<std::string, std::string>& tags() const noexcept { return tags_; }
	[[nodiscard]] const std::map<std::string, std::string>& entity_path() const noexcept { return entity_path_; }
	[[nodiscard]] const std::vector<ArgDefinition>& parameters() const noexcept { return parameters_; }
	[[nodiscard]] const std::vector<ArgDefinition>& return_values() const noexcept { return return_values_; }
	[[nodiscard]] int overload_index() const noexcept { return overload_index_; }

	// Setters (fluent API)
	FunctionDefinition& set_name(const std::string& name) { name_ = name; return *this; }
	FunctionDefinition& set_comment(const std::string& comment) { comment_ = comment; return *this; }
	FunctionDefinition& set_overload_index(int index) { overload_index_ = index; return *this; }

	// Tag management
	FunctionDefinition& set_tag(const std::string& key, const std::string& value) {
		tags_[key] = value;
		return *this;
	}

	[[nodiscard]] std::string get_tag(const std::string& key) const {
		auto it = tags_.find(key);
		return it != tags_.end() ? it->second : "";
	}

	// Entity path management
	FunctionDefinition& set_entity_path(const std::string& key, const std::string& value) {
		entity_path_[key] = value;
		return *this;
	}

	[[nodiscard]] std::string get_entity_path(const std::string& key) const {
		auto it = entity_path_.find(key);
		return it != entity_path_.end() ? it->second : "";
	}

	// Comment management
	FunctionDefinition& append_comment(const std::string& additional_comment) {
		if (!comment_.empty() && !additional_comment.empty()) {
			comment_ += "\n";
		}
		comment_ += additional_comment;
		return *this;
	}

	// Parameter/return value management (builder pattern)
	FunctionDefinition& add_parameter(const ArgDefinition& param) {
		parameters_.push_back(param);
		return *this;
	}

	FunctionDefinition& add_parameter(ArgDefinition&& param) {
		parameters_.push_back(std::move(param));
		return *this;
	}

	FunctionDefinition& add_return_value(const ArgDefinition& ret_val) {
		return_values_.push_back(ret_val);
		return *this;
	}

	FunctionDefinition& add_return_value(ArgDefinition&& ret_val) {
		return_values_.push_back(std::move(ret_val));
		return *this;
	}

	// Entity path serialization
	// Format: "key1=val1,key2=val2,..." (sorted by key)
	// Includes implicit "metaffi_guest_lib" from IDL root
	[[nodiscard]] std::string entity_path_as_string(const IDLDefinition& idl) const;

	// JSON serialization
	[[nodiscard]] virtual nlohmann::json to_json() const {
		nlohmann::json j;
		j["name"] = name_;
		j["comment"] = comment_;
		j["tags"] = tags_;
		j["entity_path"] = entity_path_;
		j["parameters"] = nlohmann::json::array();
		j["return_values"] = nlohmann::json::array();

		// Serialize parameters
		for (const auto& param : parameters_) {
			j["parameters"].push_back(param.to_json());
		}

		// Serialize return values
		for (const auto& ret_val : return_values_) {
			j["return_values"].push_back(ret_val.to_json());
		}

		// Only include overload_index if non-zero (schema default is 0)
		if (overload_index_ != 0) {
			j["overload_index"] = overload_index_;
		}

		return j;
	}

	// JSON deserialization
	static FunctionDefinition from_json(const nlohmann::json& j) {
		FunctionDefinition func;

		// Required fields with defaults
		func.name_ = j.value("name", "");
		func.comment_ = j.value("comment", "");
		func.overload_index_ = j.value("overload_index", 0);

		// Tags (optional)
		if (j.contains("tags") && j["tags"].is_object()) {
			func.tags_ = j["tags"].get<std::map<std::string, std::string>>();
		}

		// Entity path (optional)
		if (j.contains("entity_path") && j["entity_path"].is_object()) {
			func.entity_path_ = j["entity_path"].get<std::map<std::string, std::string>>();
		}

		// Parameters (optional)
		if (j.contains("parameters") && j["parameters"].is_array()) {
			for (const auto& param_json : j["parameters"]) {
				func.parameters_.push_back(ArgDefinition::from_json(param_json));
			}
		}

		// Return values (optional)
		if (j.contains("return_values") && j["return_values"].is_array()) {
			for (const auto& ret_val_json : j["return_values"]) {
				func.return_values_.push_back(ArgDefinition::from_json(ret_val_json));
			}
		}

		// Validation: fail-fast on invalid data
		if (func.name_.empty()) {
			throw IDLException("MISSING_REQUIRED_FIELD", "FunctionDefinition requires 'name' field");
		}

		return func;
	}

	// Equality comparison
	bool operator==(const FunctionDefinition& other) const {
		return name_ == other.name_ &&
		       comment_ == other.comment_ &&
		       tags_ == other.tags_ &&
		       entity_path_ == other.entity_path_ &&
		       parameters_ == other.parameters_ &&
		       return_values_ == other.return_values_ &&
		       overload_index_ == other.overload_index_;
	}

	bool operator!=(const FunctionDefinition& other) const {
		return !(*this == other);
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const FunctionDefinition& func) {
	j = func.to_json();
}

inline void from_json(const nlohmann::json& j, FunctionDefinition& func) {
	func = FunctionDefinition::from_json(j);
}

} // namespace metaffi::idl
