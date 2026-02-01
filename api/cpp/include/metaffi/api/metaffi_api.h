#pragma once

#include <cdts_serializer/cpp/cdts_cpp_serializer.h>
#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include <runtime/xcall.h>
#include <runtime/xllr_capi_loader.h>
#include <utils/env_utils.h>
#include <utils/entity_path_parser.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace metaffi::api
{

using MetaFFITypeInfo = metaffi_type_info;
using MetaFFITypes = metaffi_types;

class MetaFFIModule;
class MetaFFIEntity;

/**
 * @brief MetaFFI runtime handle. Loads/unloads a runtime plugin and creates modules.
 *
 * Fail-fast: throws std::exception on invalid input or runtime/plugin errors.
 */
class MetaFFIRuntime
{
public:
	/**
	 * @brief Create a runtime handle for a plugin.
	 * @param runtime_plugin Runtime name (e.g., "python3" or "xllr.python3").
	 */
	explicit MetaFFIRuntime(std::string runtime_plugin);

	/// @brief Normalized runtime plugin name (always prefixed with "xllr.").
	[[nodiscard]] const std::string& runtime_plugin() const;
	/// @brief Load the runtime plugin via XLLR.
	void load_runtime_plugin() const;
	/// @brief Release the runtime plugin via XLLR.
	void release_runtime_plugin() const;

	/// @brief Create a module wrapper for a given module/package path.
	[[nodiscard]] MetaFFIModule load_module(std::string module_path) const;

private:
	std::string _runtime_plugin;
};

/**
 * @brief Module wrapper used to load entities (functions/methods/fields).
 */
class MetaFFIModule
{
public:
	/**
	 * @param runtime_plugin Runtime plugin name (normalized internally).
	 * @param module_path Module/package path.
	 */
	MetaFFIModule(std::string runtime_plugin, std::string module_path);

	/// @brief Module path provided by the user.
	[[nodiscard]] const std::string& module_path() const;
	/// @brief Runtime plugin name (normalized).
	[[nodiscard]] const std::string& runtime_plugin() const;

	/**
	 * @brief Load entity with simple MetaFFI type lists (no aliases/dimensions).
	 * @param entity_path MetaFFI entity path string.
	 */
	MetaFFIEntity load_entity(const std::string& entity_path,
								std::initializer_list<metaffi_type> params_types = {},
								std::initializer_list<metaffi_type> retvals_types = {}) const;

	/**
	 * @brief Load entity with full MetaFFITypeInfo (aliases/dimensions supported).
	 */
	MetaFFIEntity load_entity_with_info(const std::string& entity_path,
										const std::vector<MetaFFITypeInfo>& params_types,
										const std::vector<MetaFFITypeInfo>& retvals_types) const;

private:
	std::string _runtime_plugin;
	std::string _module_path;
};

/**
 * @brief Loaded entity wrapper (RAII over xcall).
 *
 * - Validates parameter/retval counts and types at call time.
 * - Frees xcall on destruction (fail-fast logs if free fails).
 */
class MetaFFIEntity
{
public:
	/**
	 * @param runtime_plugin Runtime plugin name (normalized).
	 * @param pxcall XLLR xcall pointer (owned by this object).
	 * @param params_types Parameter type info.
	 * @param retvals_types Return value type info.
	 */
	MetaFFIEntity(std::string runtime_plugin,
				xcall* pxcall,
				std::vector<MetaFFITypeInfo> params_types,
				std::vector<MetaFFITypeInfo> retvals_types);

	/**
	 * @param runtime_plugin Runtime plugin name (normalized).
	 * @param pxcall XLLR xcall pointer.
	 * @param params_types Parameter type info.
	 * @param retvals_types Return value type info.
	 * @param owns_xcall If true, destructor calls xllr_free_xcall; otherwise, borrows xcall.
	 */
	MetaFFIEntity(std::string runtime_plugin,
				xcall* pxcall,
				std::vector<MetaFFITypeInfo> params_types,
				std::vector<MetaFFITypeInfo> retvals_types,
				bool owns_xcall);

	MetaFFIEntity(const MetaFFIEntity&) = delete;
	MetaFFIEntity& operator=(const MetaFFIEntity&) = delete;
	MetaFFIEntity(MetaFFIEntity&& other) noexcept;
	MetaFFIEntity& operator=(MetaFFIEntity&& other) noexcept;
	~MetaFFIEntity();

	/// @brief Parameter types as provided at load time.
	[[nodiscard]] const std::vector<MetaFFITypeInfo>& parameters_types() const;
	/// @brief Return value types as provided at load time.
	[[nodiscard]] const std::vector<MetaFFITypeInfo>& retval_types() const;

	/**
	 * @brief Call entity with C++ values and get raw CDTS return values.
	 * @throws std::invalid_argument on param count/type mismatch.
	 */
	template<typename... Args>
	cdts call_cdts(Args&&... args)
	{
		ensure_params_count(sizeof...(Args));

		cdts params(static_cast<metaffi_size>(sizeof...(Args)));
		metaffi::utils::cdts_cpp_serializer serializer(params);
		if constexpr (sizeof...(Args) > 0)
		{
			(serializer << ... << std::forward<Args>(args));
		}

		validate_params(params);
		return call_with_cdts(std::move(params));
	}

	/**
	 * @brief Call entity and deserialize return values into a tuple.
	 * @tparam Ret Return types in order (count must match retvals_types).
	 * @throws std::invalid_argument on param/retval count mismatch.
	 */
	template<typename... Ret, typename... Args>
	std::tuple<Ret...> call(Args&&... args)
	{
		ensure_retvals_count(sizeof...(Ret));

		cdts retvals = call_cdts(std::forward<Args>(args)...);
		std::tuple<Ret...> result{};

		if constexpr (sizeof...(Ret) > 0)
		{
			metaffi::utils::cdts_cpp_serializer serializer(retvals);
			std::apply(
				[&serializer](auto&... items)
				{
					(serializer >> ... >> items);
				},
				result);
		}

		return result;
	}

	/**
	 * @brief Call entity via operator() (same as call()).
	 */
	template<typename... Ret, typename... Args>
	std::tuple<Ret...> operator()(Args&&... args)
	{
		return call<Ret...>(std::forward<Args>(args)...);
	}

	/**
	 * @brief Call entity with pre-built CDTS parameters.
	 * @throws std::invalid_argument on param count/type mismatch.
	 */
	cdts call_raw(cdts&& params);

private:
	void ensure_params_count(std::size_t count) const;
	void ensure_retvals_count(std::size_t count) const;
	void validate_params(const cdts& params) const;
	void validate_retvals(const cdts& retvals) const;
	cdts call_with_cdts(cdts&& params);

	std::string _runtime_plugin;
	xcall* _pxcall;
	std::vector<MetaFFITypeInfo> _params_types;
	std::vector<MetaFFITypeInfo> _retvals_types;
	bool _owns_xcall = true;
};

/**
 * @brief Owns a MetaFFI callable (cdt_metaffi_callable) and provides a C++ entity view.
 *
 * This class owns only the callable metadata (types arrays + struct) and does not
 * own the underlying xcall pointer.
 */
class MetaFFICallable
{
public:
	explicit MetaFFICallable(cdt_metaffi_callable* callable, std::string runtime_plugin);

	MetaFFICallable(const MetaFFICallable&) = delete;
	MetaFFICallable& operator=(const MetaFFICallable&) = delete;
	MetaFFICallable(MetaFFICallable&& other) noexcept;
	MetaFFICallable& operator=(MetaFFICallable&& other) noexcept;
	~MetaFFICallable();

	[[nodiscard]] cdt_metaffi_callable* get() const;
	[[nodiscard]] bool is_null() const;
	cdt_metaffi_callable* release();

	/**
	 * @brief Call callable and deserialize return values into a tuple.
	 * @tparam Ret Return types in order (count must match callable signature).
	 */
	template<typename... Ret, typename... Args>
	std::tuple<Ret...> call(Args&&... args) const
	{
		MetaFFIEntity entity = as_entity();
		return entity.call<Ret...>(std::forward<Args>(args)...);
	}

	/**
	 * @brief Call callable via operator() (same as call()).
	 */
	template<typename... Ret, typename... Args>
	std::tuple<Ret...> operator()(Args&&... args) const
	{
		return call<Ret...>(std::forward<Args>(args)...);
	}

private:
	/// @brief Create a non-owning entity view over the callable's xcall.
	[[nodiscard]] MetaFFIEntity as_entity() const;

	cdt_metaffi_callable* _callable = nullptr;
	std::string _runtime_plugin;
};

} // namespace metaffi::api
