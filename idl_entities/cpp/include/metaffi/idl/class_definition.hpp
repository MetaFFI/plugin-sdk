#pragma once

#include "constructor_definition.hpp"
#include "method_definition.hpp"
#include "field_definition.hpp"
#include "release_definition.hpp"
#include <memory>

namespace metaffi::idl {

/**
 * ClassDefinition represents a class/struct in the IDL
 * Contains constructors, methods, fields, optional release, and metadata
 */
class ClassDefinition {
protected:
	std::string name_;
	std::string comment_;
	std::map<std::string, std::string> tags_;
	std::map<std::string, std::string> entity_path_;
	std::vector<ConstructorDefinition> constructors_;
	std::unique_ptr<ReleaseDefinition> release_;       // Nullable: classes don't require destructors
	std::vector<MethodDefinition> methods_;
	std::vector<FieldDefinition> fields_;

public:
	// Constructors
	ClassDefinition() = default;

	explicit ClassDefinition(std::string name)
		: name_(std::move(name))
	{}

	// Rule of 5 (unique_ptr requires explicit handling)
	~ClassDefinition() = default;
	ClassDefinition(const ClassDefinition& other)
		: name_(other.name_)
		, comment_(other.comment_)
		, tags_(other.tags_)
		, entity_path_(other.entity_path_)
		, constructors_(other.constructors_)
		, release_(other.release_ ? std::make_unique<ReleaseDefinition>(*other.release_) : nullptr)
		, methods_(other.methods_)
		, fields_(other.fields_)
	{}

	ClassDefinition(ClassDefinition&&) noexcept = default;

	ClassDefinition& operator=(const ClassDefinition& other) {
		if (this != &other) {
			name_ = other.name_;
			comment_ = other.comment_;
			tags_ = other.tags_;
			entity_path_ = other.entity_path_;
			constructors_ = other.constructors_;
			release_ = other.release_ ? std::make_unique<ReleaseDefinition>(*other.release_) : nullptr;
			methods_ = other.methods_;
			fields_ = other.fields_;
		}
		return *this;
	}

	ClassDefinition& operator=(ClassDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const std::string& name() const noexcept { return name_; }
	[[nodiscard]] const std::string& comment() const noexcept { return comment_; }
	[[nodiscard]] const std::map<std::string, std::string>& tags() const noexcept { return tags_; }
	[[nodiscard]] const std::map<std::string, std::string>& entity_path() const noexcept { return entity_path_; }
	[[nodiscard]] const std::vector<ConstructorDefinition>& constructors() const noexcept { return constructors_; }
	[[nodiscard]] const ReleaseDefinition* release() const noexcept { return release_.get(); }
	[[nodiscard]] const std::vector<MethodDefinition>& methods() const noexcept { return methods_; }
	[[nodiscard]] const std::vector<FieldDefinition>& fields() const noexcept { return fields_; }

	// Setters (fluent API)
	ClassDefinition& set_name(const std::string& name) { name_ = name; return *this; }
	ClassDefinition& set_comment(const std::string& comment) { comment_ = comment; return *this; }

	// Tag management
	ClassDefinition& set_tag(const std::string& key, const std::string& value) {
		tags_[key] = value;
		return *this;
	}

	[[nodiscard]] std::string get_tag(const std::string& key) const {
		auto it = tags_.find(key);
		return it != tags_.end() ? it->second : "";
	}

	// Entity path management
	ClassDefinition& set_entity_path(const std::string& key, const std::string& value) {
		entity_path_[key] = value;
		return *this;
	}

	[[nodiscard]] std::string get_entity_path(const std::string& key) const {
		auto it = entity_path_.find(key);
		return it != entity_path_.end() ? it->second : "";
	}

	// Comment management
	ClassDefinition& append_comment(const std::string& additional_comment) {
		if (!comment_.empty() && !additional_comment.empty()) {
			comment_ += "\n";
		}
		comment_ += additional_comment;
		return *this;
	}

	// Constructor/method/field management (builder pattern)
	ClassDefinition& add_constructor(const ConstructorDefinition& ctor) {
		constructors_.push_back(ctor);
		return *this;
	}

	ClassDefinition& add_constructor(ConstructorDefinition&& ctor) {
		constructors_.push_back(std::move(ctor));
		return *this;
	}

	ClassDefinition& set_release(std::unique_ptr<ReleaseDefinition> release) {
		release_ = std::move(release);
		return *this;
	}

	ClassDefinition& add_method(const MethodDefinition& method) {
		methods_.push_back(method);
		return *this;
	}

	ClassDefinition& add_method(MethodDefinition&& method) {
		methods_.push_back(std::move(method));
		return *this;
	}

	ClassDefinition& add_field(const FieldDefinition& field) {
		fields_.push_back(field);
		return *this;
	}

	ClassDefinition& add_field(FieldDefinition&& field) {
		fields_.push_back(std::move(field));
		return *this;
	}

	// JSON serialization
	[[nodiscard]] nlohmann::json to_json() const {
		nlohmann::json j;
		j["name"] = name_;
		j["comment"] = comment_;
		j["tags"] = tags_;
		j["entity_path"] = entity_path_;
		j["constructors"] = nlohmann::json::array();
		j["methods"] = nlohmann::json::array();
		j["fields"] = nlohmann::json::array();

		// Serialize constructors
		for (const auto& ctor : constructors_) {
			j["constructors"].push_back(ctor.to_json());
		}

		// Serialize release (nullable)
		if (release_) {
			j["release"] = release_->to_json();
		} else {
			j["release"] = nullptr;
		}

		// Serialize methods
		for (const auto& method : methods_) {
			j["methods"].push_back(method.to_json());
		}

		// Serialize fields
		for (const auto& field : fields_) {
			j["fields"].push_back(field.to_json());
		}

		return j;
	}

	// JSON deserialization
	static ClassDefinition from_json(const nlohmann::json& j) {
		ClassDefinition cls;

		// Required fields with defaults
		cls.name_ = j.value("name", "");
		cls.comment_ = j.value("comment", "");

		// Tags (optional)
		if (j.contains("tags") && j["tags"].is_object()) {
			cls.tags_ = j["tags"].get<std::map<std::string, std::string>>();
		}

		// Entity path (optional)
		if (j.contains("entity_path") && j["entity_path"].is_object()) {
			cls.entity_path_ = j["entity_path"].get<std::map<std::string, std::string>>();
		}

		// Constructors (optional)
		if (j.contains("constructors") && j["constructors"].is_array()) {
			for (const auto& ctor_json : j["constructors"]) {
				cls.constructors_.push_back(ConstructorDefinition::from_json(ctor_json));
			}
		}

		// Release (nullable)
		if (j.contains("release") && !j["release"].is_null()) {
			cls.release_ = std::make_unique<ReleaseDefinition>(ReleaseDefinition::from_json(j["release"]));
		}

		// Methods (optional)
		if (j.contains("methods") && j["methods"].is_array()) {
			for (const auto& method_json : j["methods"]) {
				cls.methods_.push_back(MethodDefinition::from_json(method_json));
			}
		}

		// Fields (optional)
		if (j.contains("fields") && j["fields"].is_array()) {
			for (const auto& field_json : j["fields"]) {
				cls.fields_.push_back(FieldDefinition::from_json(field_json));
			}
		}

		// Validation: fail-fast on invalid data
		if (cls.name_.empty()) {
			throw IDLException("MISSING_REQUIRED_FIELD", "ClassDefinition requires 'name' field");
		}

		return cls;
	}

	// Equality comparison
	bool operator==(const ClassDefinition& other) const {
		bool release_equal = (release_ == nullptr && other.release_ == nullptr) ||
		                     (release_ != nullptr && other.release_ != nullptr && *release_ == *other.release_);

		return name_ == other.name_ &&
		       comment_ == other.comment_ &&
		       tags_ == other.tags_ &&
		       entity_path_ == other.entity_path_ &&
		       constructors_ == other.constructors_ &&
		       release_equal &&
		       methods_ == other.methods_ &&
		       fields_ == other.fields_;
	}

	bool operator!=(const ClassDefinition& other) const {
		return !(*this == other);
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const ClassDefinition& cls) {
	j = cls.to_json();
}

inline void from_json(const nlohmann::json& j, ClassDefinition& cls) {
	cls = ClassDefinition::from_json(j);
}

} // namespace metaffi::idl
