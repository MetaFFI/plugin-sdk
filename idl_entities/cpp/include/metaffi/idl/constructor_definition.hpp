#pragma once

#include "function_definition.hpp"

namespace metaffi::idl {

// Forward declaration
class ClassDefinition;

/**
 * ConstructorDefinition represents a class constructor in the IDL
 * Extends FunctionDefinition with parent class reference
 * Constructors automatically set return value as class instance handle
 */
class ConstructorDefinition : public FunctionDefinition {
protected:
	const ClassDefinition* parent_class_ = nullptr; // Non-owning pointer to parent class

public:
	// Constructors
	ConstructorDefinition() = default;

	explicit ConstructorDefinition(std::string name)
		: FunctionDefinition(std::move(name))
	{}

	ConstructorDefinition(const ClassDefinition* parent, std::string name)
		: FunctionDefinition(std::move(name))
		, parent_class_(parent)
	{}

	// Rule of 5
	~ConstructorDefinition() override = default;
	ConstructorDefinition(const ConstructorDefinition&) = default;
	ConstructorDefinition(ConstructorDefinition&&) noexcept = default;
	ConstructorDefinition& operator=(const ConstructorDefinition&) = default;
	ConstructorDefinition& operator=(ConstructorDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const ClassDefinition* parent_class() const noexcept { return parent_class_; }

	// Setters (fluent API)
	ConstructorDefinition& set_parent_class(const ClassDefinition* parent) {
		parent_class_ = parent;
		return *this;
	}

	// JSON serialization (inherits from FunctionDefinition)
	// No additional fields beyond FunctionDefinition

	// JSON deserialization
	static ConstructorDefinition from_json(const nlohmann::json& j) {
		ConstructorDefinition ctor;

		// Deserialize base FunctionDefinition fields
		auto base_func = FunctionDefinition::from_json(j);
		ctor.name_ = base_func.name();
		ctor.comment_ = base_func.comment();
		ctor.tags_ = base_func.tags();
		ctor.entity_path_ = base_func.entity_path();
		ctor.parameters_ = base_func.parameters();
		ctor.return_values_ = base_func.return_values();
		ctor.overload_index_ = base_func.overload_index();

		return ctor;
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const ConstructorDefinition& ctor) {
	j = ctor.to_json();
}

inline void from_json(const nlohmann::json& j, ConstructorDefinition& ctor) {
	ctor = ConstructorDefinition::from_json(j);
}

} // namespace metaffi::idl
