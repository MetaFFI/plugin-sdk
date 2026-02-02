#pragma once

#include "arg_definition.hpp"
#include "function_definition.hpp"
#include <memory>

namespace metaffi::idl {

/**
 * GlobalDefinition represents a module-level global variable in the IDL
 * Extends ArgDefinition with optional getter/setter functions
 */
class GlobalDefinition : public ArgDefinition {
protected:
	std::unique_ptr<FunctionDefinition> getter_;
	std::unique_ptr<FunctionDefinition> setter_;

public:
	// Constructors
	GlobalDefinition() = default;

	GlobalDefinition(std::string name, std::string type, std::string type_alias = "")
		: ArgDefinition(std::move(name), std::move(type), std::move(type_alias))
	{}

	// Rule of 5 (unique_ptr requires explicit handling)
	~GlobalDefinition() override = default;
	GlobalDefinition(const GlobalDefinition& other)
		: ArgDefinition(other)
		, getter_(other.getter_ ? std::make_unique<FunctionDefinition>(*other.getter_) : nullptr)
		, setter_(other.setter_ ? std::make_unique<FunctionDefinition>(*other.setter_) : nullptr)
	{}

	GlobalDefinition(GlobalDefinition&&) noexcept = default;

	GlobalDefinition& operator=(const GlobalDefinition& other) {
		if (this != &other) {
			ArgDefinition::operator=(other);
			getter_ = other.getter_ ? std::make_unique<FunctionDefinition>(*other.getter_) : nullptr;
			setter_ = other.setter_ ? std::make_unique<FunctionDefinition>(*other.setter_) : nullptr;
		}
		return *this;
	}

	GlobalDefinition& operator=(GlobalDefinition&&) noexcept = default;

	// Getters
	[[nodiscard]] const FunctionDefinition* getter() const noexcept { return getter_.get(); }
	[[nodiscard]] const FunctionDefinition* setter() const noexcept { return setter_.get(); }

	// Setters (fluent API)
	GlobalDefinition& set_getter(std::unique_ptr<FunctionDefinition> getter) {
		getter_ = std::move(getter);
		return *this;
	}

	GlobalDefinition& set_setter(std::unique_ptr<FunctionDefinition> setter) {
		setter_ = std::move(setter);
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
	static GlobalDefinition from_json(const nlohmann::json& j) {
		GlobalDefinition global;

		// Deserialize base ArgDefinition fields
		auto base_arg = ArgDefinition::from_json(j);
		global.name_ = base_arg.name();
		global.type_ = base_arg.type();
		global.type_alias_ = base_arg.type_alias();
		global.comment_ = base_arg.comment();
		global.tags_ = base_arg.tags();
		global.dimensions_ = base_arg.dimensions();
		global.is_optional_ = base_arg.is_optional();

		// GlobalDefinition-specific fields
		// Getter (nullable)
		if (j.contains("getter") && !j["getter"].is_null()) {
			global.getter_ = std::make_unique<FunctionDefinition>(FunctionDefinition::from_json(j["getter"]));
		}

		// Setter (nullable)
		if (j.contains("setter") && !j["setter"].is_null()) {
			global.setter_ = std::make_unique<FunctionDefinition>(FunctionDefinition::from_json(j["setter"]));
		}

		return global;
	}

	// Equality comparison
	bool operator==(const GlobalDefinition& other) const {
		bool getters_equal = (getter_ == nullptr && other.getter_ == nullptr) ||
		                     (getter_ != nullptr && other.getter_ != nullptr && *getter_ == *other.getter_);

		bool setters_equal = (setter_ == nullptr && other.setter_ == nullptr) ||
		                     (setter_ != nullptr && other.setter_ != nullptr && *setter_ == *other.setter_);

		return ArgDefinition::operator==(other) &&
		       getters_equal &&
		       setters_equal;
	}

	bool operator!=(const GlobalDefinition& other) const {
		return !(*this == other);
	}
};

// ADL support for nlohmann/json
inline void to_json(nlohmann::json& j, const GlobalDefinition& global) {
	j = global.to_json();
}

inline void from_json(const nlohmann::json& j, GlobalDefinition& global) {
	global = GlobalDefinition::from_json(j);
}

} // namespace metaffi::idl
