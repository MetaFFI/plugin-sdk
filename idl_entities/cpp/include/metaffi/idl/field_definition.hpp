#pragma once

#include "arg_definition.hpp"
#include "method_definition.hpp"
#include <memory>

namespace metaffi::idl {

// Forward declaration
class ClassDefinition;

/**
 * FieldDefinition represents a class field/property in the IDL
 * Extends ArgDefinition with optional getter/setter methods and parent class reference
 */
class FieldDefinition : public ArgDefinition {
protected:
	std::unique_ptr<MethodDefinition> getter_;
	std::unique_ptr<MethodDefinition> setter_;
	const ClassDefinition* parent_class_ = nullptr; // Non-owning pointer to parent class

public:
	// Constructors
	FieldDefinition() = default;

	FieldDefinition(std::string name, std::string type, std::string type_alias = "")
		: ArgDefinition(std::move(name), std::move(type), std::move(type_alias))
	{}

	// Rule of 5 (unique_ptr requires explicit handling)
	~FieldDefinition() override = default;
	FieldDefinition(const FieldDefinition& other)
		: ArgDefinition(other)
		, getter_(other.getter_ ? std::make_unique<MethodDefinition>(*other.getter_) : nullptr)
		, setter_(other.setter_ ? std::make_unique<MethodDefinition>(*other.setter_) : nullptr)
		, parent_class_(other.parent_class_)
	{}

	FieldDefinition(FieldDefinition&&) noexcept = default;

	FieldDefinition& operator=(const FieldDefinition& other) {
		if (this != &other) {
			ArgDefinition::operator=(other);
			getter_ = other.getter_ ? std::make_unique<MethodDefinition>(*other.getter_) : nullptr;
			setter_ = other.setter_ ? std::make_unique<MethodDefinition>(*other.setter_) : nullptr;
			parent_class_ = other.parent_class_;
		}
		return *this;
	}

	FieldDefinition& operator=(FieldDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const MethodDefinition* getter() const noexcept { return getter_.get(); }
	[[nodiscard]] const MethodDefinition* setter() const noexcept { return setter_.get(); }
	[[nodiscard]] const ClassDefinition* parent_class() const noexcept { return parent_class_; }

	// Setters (fluent API)
	FieldDefinition& set_getter(std::unique_ptr<MethodDefinition> getter) {
		getter_ = std::move(getter);
		return *this;
	}

	FieldDefinition& set_setter(std::unique_ptr<MethodDefinition> setter) {
		setter_ = std::move(setter);
		return *this;
	}

	FieldDefinition& set_parent_class(const ClassDefinition* parent) {
		parent_class_ = parent;
		return *this;
	}

	// JSON serialization
	[[nodiscard]] nlohmann::json to_json() const {
		auto j = ArgDefinition::to_json();

		// Add getter (nullable)
		if (getter_) {
			j["getter"] = getter_->to_json();
		} else {
			j["getter"] = nullptr;
		}

		// Add setter (nullable)
		if (setter_) {
			j["setter"] = setter_->to_json();
		} else {
			j["setter"] = nullptr;
		}

		return j;
	}

	// JSON deserialization
	static FieldDefinition from_json(const nlohmann::json& j) {
		FieldDefinition field;

		// Deserialize base ArgDefinition fields
		auto base_arg = ArgDefinition::from_json(j);
		field.name_ = base_arg.name();
		field.type_ = base_arg.type();
		field.type_alias_ = base_arg.type_alias();
		field.comment_ = base_arg.comment();
		field.tags_ = base_arg.tags();
		field.dimensions_ = base_arg.dimensions();
		field.is_optional_ = base_arg.is_optional();

		// FieldDefinition-specific fields
		// Getter (nullable)
		if (j.contains("getter") && !j["getter"].is_null()) {
			field.getter_ = std::make_unique<MethodDefinition>(MethodDefinition::from_json(j["getter"]));
		}

		// Setter (nullable)
		if (j.contains("setter") && !j["setter"].is_null()) {
			field.setter_ = std::make_unique<MethodDefinition>(MethodDefinition::from_json(j["setter"]));
		}

		return field;
	}

	// Equality comparison
	bool operator==(const FieldDefinition& other) const {
		bool getters_equal = (getter_ == nullptr && other.getter_ == nullptr) ||
		                     (getter_ != nullptr && other.getter_ != nullptr && *getter_ == *other.getter_);

		bool setters_equal = (setter_ == nullptr && other.setter_ == nullptr) ||
		                     (setter_ != nullptr && other.setter_ != nullptr && *setter_ == *other.setter_);

		return ArgDefinition::operator==(other) &&
		       getters_equal &&
		       setters_equal;
	}

	bool operator!=(const FieldDefinition& other) const {
		return !(*this == other);
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const FieldDefinition& field) {
	j = field.to_json();
}

inline void from_json(const nlohmann::json& j, FieldDefinition& field) {
	field = FieldDefinition::from_json(j);
}

} // namespace metaffi::idl
