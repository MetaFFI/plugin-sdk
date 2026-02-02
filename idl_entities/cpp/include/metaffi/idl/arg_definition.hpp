#pragma once

#include "metaffi_type.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <map>

namespace metaffi::idl {

/**
 * ArgDefinition represents a parameter or return value in the IDL
 * Used by functions, methods, constructors, and as a base for fields and globals
 */
class ArgDefinition {
protected:
	std::string name_;
	std::string type_;                              // MetaFFI type (e.g., "int64", "string8")
	std::string type_alias_;                        // Language-specific type (e.g., "int64_t", "std::string")
	std::string comment_;
	std::map<std::string, std::string> tags_;
	int dimensions_ = 0;                            // Array dimensions (0 = scalar, >0 = array)
	bool is_optional_ = false;

public:
	// Constructors (RAII-compliant with automatic cleanup)
	ArgDefinition() = default;

	ArgDefinition(std::string name, std::string type, std::string type_alias = "",
	              std::string comment = "", int dimensions = 0, bool is_optional = false)
		: name_(std::move(name))
		, type_(std::move(type))
		, type_alias_(std::move(type_alias))
		, comment_(std::move(comment))
		, dimensions_(dimensions)
		, is_optional_(is_optional)
	{}

	// Rule of 5 (default implementations are fine due to RAII members)
	virtual ~ArgDefinition() = default;
	ArgDefinition(const ArgDefinition&) = default;
	ArgDefinition(ArgDefinition&&) noexcept = default;
	ArgDefinition& operator=(const ArgDefinition&) = default;
	ArgDefinition& operator=(ArgDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const std::string& name() const noexcept { return name_; }
	[[nodiscard]] const std::string& type() const noexcept { return type_; }
	[[nodiscard]] const std::string& type_alias() const noexcept { return type_alias_; }
	[[nodiscard]] const std::string& comment() const noexcept { return comment_; }
	[[nodiscard]] const std::map<std::string, std::string>& tags() const noexcept { return tags_; }
	[[nodiscard]] int dimensions() const noexcept { return dimensions_; }
	[[nodiscard]] bool is_optional() const noexcept { return is_optional_; }

	// Setters (fluent API)
	ArgDefinition& set_name(const std::string& name) { name_ = name; return *this; }
	ArgDefinition& set_type(const std::string& type) { type_ = type; return *this; }
	ArgDefinition& set_type_alias(const std::string& alias) { type_alias_ = alias; return *this; }
	ArgDefinition& set_comment(const std::string& comment) { comment_ = comment; return *this; }
	ArgDefinition& set_dimensions(int dims) { dimensions_ = dims; return *this; }
	ArgDefinition& set_optional(bool optional) { is_optional_ = optional; return *this; }

	// Tag management
	ArgDefinition& set_tag(const std::string& key, const std::string& value) {
		tags_[key] = value;
		return *this;
	}

	[[nodiscard]] std::string get_tag(const std::string& key) const {
		auto it = tags_.find(key);
		return it != tags_.end() ? it->second : "";
	}

	// Comment management
	ArgDefinition& append_comment(const std::string& additional_comment) {
		if (!comment_.empty() && !additional_comment.empty()) {
			comment_ += "\n";
		}
		comment_ += additional_comment;
		return *this;
	}

	// Helper methods
	[[nodiscard]] bool is_array() const noexcept {
		return dimensions_ > 0 || type_.find("_array") != std::string::npos;
	}

	[[nodiscard]] bool is_handle() const noexcept {
		return type_ == "handle" || type_ == "handle_array";
	}

	[[nodiscard]] bool is_string() const noexcept {
		return type_ == "string8" || type_ == "string16" || type_ == "string32" ||
		       type_ == "string8_array" || type_ == "string16_array" || type_ == "string32_array";
	}

	// JSON serialization
	[[nodiscard]] nlohmann::json to_json() const {
		nlohmann::json j;
		j["name"] = name_;
		j["type"] = type_;
		j["type_alias"] = type_alias_;
		j["comment"] = comment_;
		j["tags"] = tags_;
		j["dimensions"] = dimensions_;

		// Only include is_optional if true (schema default is false)
		if (is_optional_) {
			j["is_optional"] = true;
		}

		return j;
	}

	// JSON deserialization
	static ArgDefinition from_json(const nlohmann::json& j) {
		ArgDefinition arg;

		// Required fields with defaults
		arg.name_ = j.value("name", "");
		arg.type_ = j.value("type", "");
		arg.type_alias_ = j.value("type_alias", "");
		arg.comment_ = j.value("comment", "");
		arg.dimensions_ = j.value("dimensions", 0);
		arg.is_optional_ = j.value("is_optional", false);

		// Tags (optional)
		if (j.contains("tags") && j["tags"].is_object()) {
			arg.tags_ = j["tags"].get<std::map<std::string, std::string>>();
		}

		// Validation: fail-fast on invalid data
		if (arg.name_.empty()) {
			throw IDLException("MISSING_REQUIRED_FIELD", "ArgDefinition requires 'name' field",
			                   "type: " + arg.type_);
		}
		if (arg.type_.empty()) {
			throw IDLException("MISSING_REQUIRED_FIELD", "ArgDefinition requires 'type' field",
			                   "name: " + arg.name_);
		}

		return arg;
	}

	// Equality comparison (useful for testing)
	bool operator==(const ArgDefinition& other) const {
		return name_ == other.name_ &&
		       type_ == other.type_ &&
		       type_alias_ == other.type_alias_ &&
		       comment_ == other.comment_ &&
		       tags_ == other.tags_ &&
		       dimensions_ == other.dimensions_ &&
		       is_optional_ == other.is_optional_;
	}

	bool operator!=(const ArgDefinition& other) const {
		return !(*this == other);
	}
};

// ADL (Argument-Dependent Lookup) support for nlohmann/json
inline void to_json(nlohmann::json& j, const ArgDefinition& arg) {
	j = arg.to_json();
}

inline void from_json(const nlohmann::json& j, ArgDefinition& arg) {
	arg = ArgDefinition::from_json(j);
}

} // namespace metaffi::idl
