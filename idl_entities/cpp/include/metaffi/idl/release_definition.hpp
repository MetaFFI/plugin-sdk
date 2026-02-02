#pragma once

#include "method_definition.hpp"

namespace metaffi::idl {

/**
 * ReleaseDefinition represents a class destructor/releaser in the IDL
 * Extends MethodDefinition for resource cleanup methods
 */
class ReleaseDefinition : public MethodDefinition {
public:
	// Constructors
	ReleaseDefinition() = default;

	explicit ReleaseDefinition(std::string name)
		: MethodDefinition(std::move(name), true) // Release always requires instance
	{}

	ReleaseDefinition(const ClassDefinition* parent, std::string name)
		: MethodDefinition(parent, std::move(name), true)
	{}

	// Rule of 5
	~ReleaseDefinition() override = default;
	ReleaseDefinition(const ReleaseDefinition&) = default;
	ReleaseDefinition(ReleaseDefinition&&) noexcept = default;
	ReleaseDefinition& operator=(const ReleaseDefinition&) = default;
	ReleaseDefinition& operator=(ReleaseDefinition&&) noexcept = default;

	// JSON serialization (inherits from MethodDefinition)
	// No additional fields beyond MethodDefinition

	// JSON deserialization
	static ReleaseDefinition from_json(const nlohmann::json& j) {
		ReleaseDefinition release;

		// Deserialize base MethodDefinition fields
		auto base_method = MethodDefinition::from_json(j);
		release.name_ = base_method.name();
		release.comment_ = base_method.comment();
		release.tags_ = base_method.tags();
		release.entity_path_ = base_method.entity_path();
		release.parameters_ = base_method.parameters();
		release.return_values_ = base_method.return_values();
		release.overload_index_ = base_method.overload_index();
		release.instance_required_ = base_method.instance_required();

		return release;
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const ReleaseDefinition& release) {
	j = release.to_json();
}

inline void from_json(const nlohmann::json& j, ReleaseDefinition& release) {
	release = ReleaseDefinition::from_json(j);
}

} // namespace metaffi::idl
