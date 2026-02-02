#pragma once

#include "function_definition.hpp"

namespace metaffi::idl {

// Forward declaration
class ClassDefinition;

/**
 * MethodDefinition represents a class method in the IDL
 * Extends FunctionDefinition with instance_required flag and parent class reference
 */
class MethodDefinition : public FunctionDefinition {
protected:
	bool instance_required_ = true;                // false for static methods
	const ClassDefinition* parent_class_ = nullptr; // Non-owning pointer to parent class

public:
	// Constructors
	MethodDefinition() = default;

	explicit MethodDefinition(std::string name, bool instance_required = true)
		: FunctionDefinition(std::move(name))
		, instance_required_(instance_required)
	{}

	MethodDefinition(const ClassDefinition* parent, std::string name, bool instance_required = true)
		: FunctionDefinition(std::move(name))
		, instance_required_(instance_required)
		, parent_class_(parent)
	{}

	// Rule of 5
	~MethodDefinition() override = default;
	MethodDefinition(const MethodDefinition&) = default;
	MethodDefinition(MethodDefinition&&) noexcept = default;
	MethodDefinition& operator=(const MethodDefinition&) = default;
	MethodDefinition& operator=(MethodDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] bool instance_required() const noexcept { return instance_required_; }
	[[nodiscard]] const ClassDefinition* parent_class() const noexcept { return parent_class_; }

	// Setters (fluent API)
	MethodDefinition& set_instance_required(bool required) {
		instance_required_ = required;
		return *this;
	}

	MethodDefinition& set_parent_class(const ClassDefinition* parent) {
		parent_class_ = parent;
		return *this;
	}

	// Entity path serialization (overrides FunctionDefinition)
	// Merges parent class entity_path with method entity_path
	[[nodiscard]] std::string entity_path_as_string(const IDLDefinition& idl) const;

	// JSON serialization (overrides FunctionDefinition)
	[[nodiscard]] nlohmann::json to_json() const override {
		auto j = FunctionDefinition::to_json();

		// Only include instance_required if false (schema default is true)
		if (!instance_required_) {
			j["instance_required"] = false;
		}

		return j;
	}

	// JSON deserialization
	static MethodDefinition from_json(const nlohmann::json& j) {
		MethodDefinition method;

		// Deserialize base FunctionDefinition fields
		auto base_func = FunctionDefinition::from_json(j);
		method.name_ = base_func.name();
		method.comment_ = base_func.comment();
		method.tags_ = base_func.tags();
		method.entity_path_ = base_func.entity_path();
		method.parameters_ = base_func.parameters();
		method.return_values_ = base_func.return_values();
		method.overload_index_ = base_func.overload_index();

		// MethodDefinition-specific fields
		method.instance_required_ = j.value("instance_required", true);

		return method;
	}

	// Equality comparison
	bool operator==(const MethodDefinition& other) const {
		return FunctionDefinition::operator==(other) &&
		       instance_required_ == other.instance_required_;
	}

	bool operator!=(const MethodDefinition& other) const {
		return !(*this == other);
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const MethodDefinition& method) {
	j = method.to_json();
}

inline void from_json(const nlohmann::json& j, MethodDefinition& method) {
	method = MethodDefinition::from_json(j);
}

} // namespace metaffi::idl
