#include "metaffi/api/metaffi_api.h"

#include <sstream>
#include <utils/logger.hpp>

namespace metaffi::api
{

namespace
{
static auto LOG = metaffi::get_logger("cpp.api");
constexpr const char* kRuntimePluginPrefix = "xllr.";

std::string normalize_runtime_plugin(std::string runtime_plugin)
{
	if(runtime_plugin.empty())
	{
		throw std::invalid_argument("runtime_plugin must not be empty");
	}

	if(runtime_plugin.rfind(kRuntimePluginPrefix, 0) == 0)
	{
		return runtime_plugin;
	}

	return std::string(kRuntimePluginPrefix) + runtime_plugin;
}

void ensure_count_fits_int8(std::size_t count, const char* label)
{
	if(count > static_cast<std::size_t>((std::numeric_limits<int8_t>::max)()))
	{
		std::ostringstream ss;
		ss << label << " count exceeds int8_t max: " << count;
		throw std::invalid_argument(ss.str());
	}
}

void throw_if_err(char* err, const char* context)
{
	if(err == nullptr)
	{
		return;
	}

	std::string err_text(err);
	xllr_free_string(err);

	std::string msg = context;
	msg += ": ";
	msg += err_text;
	throw std::runtime_error(msg);
}

bool has_any_flag(metaffi_type type)
{
	return (type & metaffi_any_type) == metaffi_any_type;
}

bool is_array_type(metaffi_type type)
{
	return (type & metaffi_array_type) == metaffi_array_type;
}

metaffi_type base_type(metaffi_type type)
{
	return is_array_type(type) ? (type & ~metaffi_array_type) : type;
}

bool matches_expected_type(const MetaFFITypeInfo& expected, const cdt& actual)
{
	const metaffi_type expected_type = expected.type;
	const metaffi_type actual_type = actual.type;

	if(expected_type == metaffi_array_type)
	{
		return is_array_type(actual_type);
	}

	if(has_any_flag(expected_type))
	{
		if(is_array_type(expected_type))
		{
			return is_array_type(actual_type);
		}
		return true;
	}

	if(is_array_type(expected_type))
	{
		if(!is_array_type(actual_type))
		{
			return false;
		}

		if(base_type(expected_type) != base_type(actual_type))
		{
			return false;
		}

		if(expected.fixed_dimensions != MIXED_OR_UNKNOWN_DIMENSIONS)
		{
			const cdts& arr = static_cast<const cdts&>(actual);
			if(arr.fixed_dimensions != expected.fixed_dimensions)
			{
				return false;
			}
		}

		return true;
	}

	return expected_type == actual_type;
}

void validate_cdts_types(const std::vector<MetaFFITypeInfo>& expected,
						 const cdts& actual,
						 const char* what)
{
	if(actual.length != expected.size())
	{
		std::ostringstream ss;
		ss << what << " count mismatch. expected=" << expected.size()
		   << ", actual=" << actual.length;
		throw std::invalid_argument(ss.str());
	}

	for(std::size_t i = 0; i < expected.size(); ++i)
	{
		const MetaFFITypeInfo& exp = expected[i];
		const cdt& value = actual.arr[i];
		if(!matches_expected_type(exp, value))
		{
			std::ostringstream ss;
			ss << what << " type mismatch at index " << i
			   << ". expected type=" << exp.type
			   << ", actual type=" << value.type;
			throw std::invalid_argument(ss.str());
		}
	}
}

} // namespace

MetaFFIRuntime::MetaFFIRuntime(std::string runtime_plugin)
	: _runtime_plugin(normalize_runtime_plugin(std::move(runtime_plugin)))
{
}

const std::string& MetaFFIRuntime::runtime_plugin() const
{
	return _runtime_plugin;
}

void MetaFFIRuntime::load_runtime_plugin() const
{
	if(get_env_var("METAFFI_HOME").empty())
	{
		throw std::runtime_error("METAFFI_HOME environment variable is not set");
	}

	char* err = nullptr;
	xllr_load_runtime_plugin(_runtime_plugin.c_str(), &err);
	throw_if_err(err, "Failed to load runtime plugin");
}

void MetaFFIRuntime::release_runtime_plugin() const
{
	char* err = nullptr;
	xllr_free_runtime_plugin(_runtime_plugin.c_str(), &err);
	throw_if_err(err, "Failed to release runtime plugin");
}

MetaFFIModule MetaFFIRuntime::load_module(std::string module_path) const
{
	return MetaFFIModule(_runtime_plugin, std::move(module_path));
}

MetaFFIModule::MetaFFIModule(std::string runtime_plugin, std::string module_path)
	: _runtime_plugin(normalize_runtime_plugin(std::move(runtime_plugin))),
	  _module_path(std::move(module_path))
{
	if(_module_path.empty())
	{
		throw std::invalid_argument("module_path must not be empty");
	}
}

const std::string& MetaFFIModule::module_path() const
{
	return _module_path;
}

const std::string& MetaFFIModule::runtime_plugin() const
{
	return _runtime_plugin;
}

MetaFFIEntity MetaFFIModule::load_entity(const std::string& entity_path,
								std::initializer_list<metaffi_type> params_types,
								std::initializer_list<metaffi_type> retvals_types) const
{
	std::vector<MetaFFITypeInfo> params;
	params.reserve(params_types.size());
	for(auto t : params_types)
	{
		params.emplace_back(t);
	}

	std::vector<MetaFFITypeInfo> retvals;
	retvals.reserve(retvals_types.size());
	for(auto t : retvals_types)
	{
		retvals.emplace_back(t);
	}

	return load_entity_with_info(entity_path, params, retvals);
}

MetaFFIEntity MetaFFIModule::load_entity_with_info(const std::string& entity_path,
									const std::vector<MetaFFITypeInfo>& params_types,
									const std::vector<MetaFFITypeInfo>& retvals_types) const
{
	if(entity_path.empty())
	{
		throw std::invalid_argument("entity_path must not be empty");
	}

	// Validate entity path format early (fail-fast)
	metaffi::utils::entity_path_parser parser(entity_path);
	(void)parser;

	ensure_count_fits_int8(params_types.size(), "params_types");
	ensure_count_fits_int8(retvals_types.size(), "retvals_types");

	std::vector<MetaFFITypeInfo> params_copy = params_types;
	std::vector<MetaFFITypeInfo> retvals_copy = retvals_types;

	char* err = nullptr;
	xcall* pxcall = xllr_load_entity(
		_runtime_plugin.c_str(),
		_module_path.c_str(),
		entity_path.c_str(),
		params_copy.empty() ? nullptr : params_copy.data(),
		static_cast<int8_t>(params_copy.size()),
		retvals_copy.empty() ? nullptr : retvals_copy.data(),
		static_cast<int8_t>(retvals_copy.size()),
		&err);

	throw_if_err(err, "Failed to load entity");

	if(pxcall == nullptr)
	{
		throw std::runtime_error("xllr_load_entity returned null xcall");
	}

	return MetaFFIEntity(_runtime_plugin, pxcall, std::move(params_copy), std::move(retvals_copy));
}

MetaFFIEntity::MetaFFIEntity(std::string runtime_plugin,
				xcall* pxcall,
				std::vector<MetaFFITypeInfo> params_types,
				std::vector<MetaFFITypeInfo> retvals_types)
	: MetaFFIEntity(std::move(runtime_plugin),
	                pxcall,
	                std::move(params_types),
	                std::move(retvals_types),
	                true)
{
}

MetaFFIEntity::MetaFFIEntity(std::string runtime_plugin,
				xcall* pxcall,
				std::vector<MetaFFITypeInfo> params_types,
				std::vector<MetaFFITypeInfo> retvals_types,
				bool owns_xcall)
	: _runtime_plugin(normalize_runtime_plugin(std::move(runtime_plugin))),
	  _pxcall(pxcall),
	  _params_types(std::move(params_types)),
	  _retvals_types(std::move(retvals_types)),
	  _owns_xcall(owns_xcall)
{
	if(_pxcall == nullptr)
	{
		throw std::invalid_argument("pxcall must not be null");
	}
}

MetaFFIEntity::MetaFFIEntity(MetaFFIEntity&& other) noexcept
	: _runtime_plugin(std::move(other._runtime_plugin)),
	  _pxcall(other._pxcall),
	  _params_types(std::move(other._params_types)),
	  _retvals_types(std::move(other._retvals_types)),
	  _owns_xcall(other._owns_xcall)
{
	other._pxcall = nullptr;
	other._owns_xcall = false;
}

MetaFFIEntity& MetaFFIEntity::operator=(MetaFFIEntity&& other) noexcept
{
	if(this != &other)
	{
		if(_pxcall != nullptr && _owns_xcall)
		{
			char* err = nullptr;
			xllr_free_xcall(_runtime_plugin.c_str(), _pxcall, &err);
			if(err)
			{
				METAFFI_ERROR(LOG, "Failed to free xcall in move assignment: {}", err);
				xllr_free_string(err);
			}
		}

		_runtime_plugin = std::move(other._runtime_plugin);
		_pxcall = other._pxcall;
		_params_types = std::move(other._params_types);
		_retvals_types = std::move(other._retvals_types);
		_owns_xcall = other._owns_xcall;
		other._pxcall = nullptr;
		other._owns_xcall = false;
	}

	return *this;
}

MetaFFIEntity::~MetaFFIEntity()
{
	if(_pxcall != nullptr && _owns_xcall)
	{
		char* err = nullptr;
		xllr_free_xcall(_runtime_plugin.c_str(), _pxcall, &err);
		if(err)
		{
			METAFFI_ERROR(LOG, "Failed to free xcall: {}", err);
			xllr_free_string(err);
		}
		_pxcall = nullptr;
	}
}

const std::vector<MetaFFITypeInfo>& MetaFFIEntity::parameters_types() const
{
	return _params_types;
}

const std::vector<MetaFFITypeInfo>& MetaFFIEntity::retval_types() const
{
	return _retvals_types;
}

void MetaFFIEntity::ensure_params_count(std::size_t count) const
{
	if(count != _params_types.size())
	{
		std::ostringstream ss;
		ss << "Parameters count mismatch. expected=" << _params_types.size()
		   << ", actual=" << count;
		throw std::invalid_argument(ss.str());
	}
}

void MetaFFIEntity::ensure_retvals_count(std::size_t count) const
{
	if(count != _retvals_types.size())
	{
		std::ostringstream ss;
		ss << "Return values count mismatch. expected=" << _retvals_types.size()
		   << ", actual=" << count;
		throw std::invalid_argument(ss.str());
	}
}

void MetaFFIEntity::validate_params(const cdts& params) const
{
	validate_cdts_types(_params_types, params, "Parameter");
}

void MetaFFIEntity::validate_retvals(const cdts& retvals) const
{
	if(_retvals_types.empty())
	{
		return;
	}
	validate_cdts_types(_retvals_types, retvals, "Return value");
}

cdts MetaFFIEntity::call_with_cdts(cdts&& params)
{
	if(_pxcall == nullptr)
	{
		throw std::runtime_error("xcall is null");
	}

	const metaffi_size params_count = params.length;
	const metaffi_size retvals_count = static_cast<metaffi_size>(_retvals_types.size());
	cdts retvals(retvals_count);

	char* err = nullptr;

	if(params_count == 0 && retvals_count == 0)
	{
		xllr_xcall_no_params_no_ret(_pxcall, &err);
		throw_if_err(err, "xcall invocation failed");
		return cdts();
	}

	if(params_count > 0 && retvals_count > 0)
	{
		std::array<cdts, 2> params_ret{};
		params_ret[0] = std::move(params);
		params_ret[1] = std::move(retvals);

		xllr_xcall_params_ret(_pxcall, params_ret.data(), &err);
		throw_if_err(err, "xcall invocation failed");

		retvals = std::move(params_ret[1]);
		validate_retvals(retvals);
		return retvals;
	}

	if(params_count > 0)
	{
		std::array<cdts, 1> params_only{};
		params_only[0] = std::move(params);

		xllr_xcall_params_no_ret(_pxcall, params_only.data(), &err);
		throw_if_err(err, "xcall invocation failed");
		return cdts();
	}

	std::array<cdts, 1> ret_only{};
	ret_only[0] = std::move(retvals);

	xllr_xcall_no_params_ret(_pxcall, ret_only.data(), &err);
	throw_if_err(err, "xcall invocation failed");

	retvals = std::move(ret_only[0]);
	validate_retvals(retvals);
	return retvals;
}

cdts MetaFFIEntity::call_raw(cdts&& params)
{
	ensure_params_count(params.length);
	validate_params(params);
	return call_with_cdts(std::move(params));
}

MetaFFICallable::MetaFFICallable(cdt_metaffi_callable* callable, std::string runtime_plugin)
	: _callable(callable),
	  _runtime_plugin(normalize_runtime_plugin(std::move(runtime_plugin)))
{
}

MetaFFICallable::MetaFFICallable(MetaFFICallable&& other) noexcept
	: _callable(other._callable),
	  _runtime_plugin(std::move(other._runtime_plugin))
{
	other._callable = nullptr;
}

MetaFFICallable& MetaFFICallable::operator=(MetaFFICallable&& other) noexcept
{
	if(this != &other)
	{
		if(_callable)
		{
			if(_callable->parameters_types)
			{
				xllr_free_memory(_callable->parameters_types);
			}
			if(_callable->retval_types)
			{
				xllr_free_memory(_callable->retval_types);
			}
			xllr_free_memory(_callable);
		}

		_callable = other._callable;
		_runtime_plugin = std::move(other._runtime_plugin);
		other._callable = nullptr;
	}
	return *this;
}

MetaFFICallable::~MetaFFICallable()
{
	if(_callable)
	{
		if(_callable->parameters_types)
		{
			xllr_free_memory(_callable->parameters_types);
		}
		if(_callable->retval_types)
		{
			xllr_free_memory(_callable->retval_types);
		}
		xllr_free_memory(_callable);
		_callable = nullptr;
	}
}

cdt_metaffi_callable* MetaFFICallable::get() const
{
	return _callable;
}

bool MetaFFICallable::is_null() const
{
	return !_callable || !_callable->val;
}

cdt_metaffi_callable* MetaFFICallable::release()
{
	cdt_metaffi_callable* released = _callable;
	_callable = nullptr;
	return released;
}

MetaFFIEntity MetaFFICallable::as_entity() const
{
	if(!_callable || !_callable->val)
	{
		throw std::invalid_argument("callable must not be null");
	}

	std::vector<MetaFFITypeInfo> params;
	params.reserve(_callable->params_types_length);
	for(int i = 0; i < _callable->params_types_length; ++i)
	{
		const metaffi_type type = _callable->parameters_types ? _callable->parameters_types[i] : metaffi_any_type;
		params.emplace_back(type);
	}

	std::vector<MetaFFITypeInfo> retvals;
	retvals.reserve(_callable->retval_types_length);
	for(int i = 0; i < _callable->retval_types_length; ++i)
	{
		const metaffi_type type = _callable->retval_types ? _callable->retval_types[i] : metaffi_any_type;
		retvals.emplace_back(type);
	}

	return MetaFFIEntity(_runtime_plugin,
	                     reinterpret_cast<xcall*>(_callable->val),
	                     std::move(params),
	                     std::move(retvals),
	                     false);
}

} // namespace metaffi::api
