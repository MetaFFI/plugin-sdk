#include "cdts_wrapper.h"
#include <mutex>


// NOLINT(bugprone-macro-parentheses)
bool capi_loaded = false;
const char* capi_loader_err = nullptr;
struct capi_loader
{
	capi_loader()
	{
		capi_loader_err = load_cdt_capi();
		if(capi_loader_err)
		{
			printf("FATAL ERROR! Failed to load CDT C-API. Error: %s\n", capi_loader_err);
		}
		
		capi_loaded = true;
	}
};
static capi_loader _l; // load statically the CDTS CAPI-API


namespace metaffi::runtime
{
//--------------------------------------------------------------------
	cdts_wrapper::cdts_wrapper(cdt* cdts, metaffi_size cdts_length, bool is_free_cdts /*= false*/):cdts(cdts),cdts_length(cdts_length), is_free_cdts(is_free_cdts)
	{
		if(!capi_loaded)
		{
			throw std::runtime_error("Failed to load CDT C-API");
		}
	}
//--------------------------------------------------------------------
	cdt* cdts_wrapper::operator[](int index) const
	{
		if(index >= this->cdts_length)
		{
			throw std::runtime_error("operator[] requested index is out of bounds");
		}
		
		return &this->cdts[index];
	}
//--------------------------------------------------------------------
	cdt* cdts_wrapper::get_cdts() const
	{
		return this->cdts;
	}
//--------------------------------------------------------------------
	metaffi_size cdts_wrapper::get_cdts_length() const
	{
		return this->cdts_length;
	}
//--------------------------------------------------------------------
	
	
	
	metaffi_type_with_alias make_type_with_alias(metaffi_type type, const std::string& alias)
	{
		metaffi_type_with_alias inst = {0};
		inst.type = type;
		
		if(!alias.empty())
		{
			inst.alias = (char*)malloc(alias.size());
			std::copy(alias.begin(), alias.end(), inst.alias);
			inst.alias_length = (int64_t)alias.length();
		}
		
		return inst;
	}
//--------------------------------------------------------------------
	void cdts_wrapper::set(int index, metaffi_float64 v) const
	{
		(*this)[index]->type = metaffi_float64_type;
		(*this)[index]->cdt_val.metaffi_float64_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_float32 v) const
	{
		(*this)[index]->type = metaffi_float32_type;
		(*this)[index]->cdt_val.metaffi_float32_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_int8 v) const
	{
		(*this)[index]->type = metaffi_int8_type;
		(*this)[index]->cdt_val.metaffi_int8_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_int16 v) const
	{
		(*this)[index]->type = metaffi_int16_type;
		(*this)[index]->cdt_val.metaffi_int16_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_int32 v) const
	{
		(*this)[index]->type = metaffi_int32_type;
		(*this)[index]->cdt_val.metaffi_int32_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_int64 v) const
	{
		(*this)[index]->type = metaffi_int64_type;
		(*this)[index]->cdt_val.metaffi_int64_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_uint8 v) const
	{
		(*this)[index]->type = metaffi_uint8_type;
		(*this)[index]->cdt_val.metaffi_uint8_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_uint16 v) const
	{
		(*this)[index]->type = metaffi_uint16_type;
		(*this)[index]->cdt_val.metaffi_uint16_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_uint32 v) const
	{
		(*this)[index]->type = metaffi_uint32_type;
		(*this)[index]->cdt_val.metaffi_uint32_val.val = v;
	}
	void cdts_wrapper::set(int index, metaffi_uint64 v) const
	{
		(*this)[index]->type = metaffi_uint64_type;
		(*this)[index]->cdt_val.metaffi_uint64_val.val = v;
	}
	
	void cdts_wrapper::set(int index, bool v) const
	{
		(*this)[index]->type = metaffi_bool_type;
		(*this)[index]->cdt_val.metaffi_bool_val.val = v ? 1 : 0;
	}
	
	void cdts_wrapper::set(int index, const std::string& v) const
	{
		(*this)[index]->type = metaffi_string8_type;
		(*this)[index]->cdt_val.metaffi_string8_val.val = (char*)calloc(1, sizeof(char*)*v.size()+1);
		std::copy(v.begin(), v.end(), (*this)[index]->cdt_val.metaffi_string8_val.val);
		(*this)[index]->cdt_val.metaffi_string8_val.length = v.length();
	}
	
	void cdts_wrapper::set(int index, metaffi_handle v, uint64_t runtime_id) const
	{
		(*this)[index]->type = metaffi_handle_type;
		(*this)[index]->cdt_val.metaffi_handle_val.val = v;
		(*this)[index]->cdt_val.metaffi_handle_val.runtime_id = runtime_id;
	}
	
	void cdts_wrapper::set(int index, const metaffi_float64* v, int length) const
	{
		(*this)[index]->type = metaffi_float64_array_type;
		(*this)[index]->cdt_val.metaffi_float64_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_float64_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_float64_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_float64_array_val.vals = (metaffi_float64*)malloc(sizeof(metaffi_float64)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_float64_array_val.vals[i] = v[i];
		}
	}
	void cdts_wrapper::set(int index, const metaffi_float32* v, int length) const
	{
		(*this)[index]->type = metaffi_float32_array_type;
		(*this)[index]->cdt_val.metaffi_float32_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_float32_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_float32_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_float32_array_val.vals = (metaffi_float32*)malloc(sizeof(metaffi_float32)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_float32_array_val.vals[i] = v[i];
		}
	}
	
	void cdts_wrapper::set(int index, const metaffi_int8* v, int length) const
	{
		(*this)[index]->type = metaffi_int8_array_type;
		(*this)[index]->cdt_val.metaffi_int8_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_int8_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_int8_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_int8_array_val.vals = (metaffi_int8*)malloc(sizeof(metaffi_int8)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_int8_array_val.vals[i] = v[i];
		}
	}
	void cdts_wrapper::set(int index, const metaffi_int16* v, int length) const
	{
		(*this)[index]->type = metaffi_int16_array_type;
		(*this)[index]->cdt_val.metaffi_int16_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_int16_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_int16_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_int16_array_val.vals = (metaffi_int16*)malloc(sizeof(metaffi_int16)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_int16_array_val.vals[i] = v[i];
		}
	}
	void cdts_wrapper::set(int index, const metaffi_int32* v, int length) const
	{
		(*this)[index]->type = metaffi_int32_array_type;
		(*this)[index]->cdt_val.metaffi_int32_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_int32_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_int32_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_int32_array_val.vals = (metaffi_int32*)malloc(sizeof(metaffi_int32)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_int32_array_val.vals[i] = v[i];
		}
	}
	void cdts_wrapper::set(int index, const metaffi_int64* v, int length) const
	{
		(*this)[index]->type = metaffi_int64_array_type;
		(*this)[index]->cdt_val.metaffi_int64_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_int64_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_int64_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_int64_array_val.vals = (metaffi_int64*)malloc(sizeof(metaffi_int64)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_int64_array_val.vals[i] = v[i];
		}
	}
	
	void cdts_wrapper::set(int index, const metaffi_uint8* v, int length) const
	{
		(*this)[index]->type = metaffi_uint8_array_type;
		(*this)[index]->cdt_val.metaffi_uint8_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_uint8_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_uint8_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_uint8_array_val.vals = (metaffi_uint8*)malloc(sizeof(metaffi_uint8)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_uint8_array_val.vals[i] = v[i];
		}
	}
	void cdts_wrapper::set(int index, const metaffi_uint16* v, int length) const
	{
		(*this)[index]->type = metaffi_uint16_array_type;
		(*this)[index]->cdt_val.metaffi_uint16_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_uint16_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_uint16_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_uint16_array_val.vals = (metaffi_uint16*)malloc(sizeof(metaffi_uint16)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_uint16_array_val.vals[i] = v[i];
		}
	}
	void cdts_wrapper::set(int index, const metaffi_uint32* v, int length) const
	{
		(*this)[index]->type = metaffi_uint32_array_type;
		(*this)[index]->cdt_val.metaffi_uint32_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_uint32_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_uint32_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_uint32_array_val.vals = (metaffi_uint32*)malloc(sizeof(metaffi_uint32)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_uint32_array_val.vals[i] = v[i];
		}
	}
	void cdts_wrapper::set(int index, const metaffi_uint64* v, int length) const
	{
		(*this)[index]->type = metaffi_uint64_array_type;
		(*this)[index]->cdt_val.metaffi_uint64_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_uint64_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_uint64_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_uint64_array_val.vals = (metaffi_uint64*)malloc(sizeof(metaffi_uint64)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_uint64_array_val.vals[i] = v[i];
		}
	}
	
	void cdts_wrapper::set(int index, const bool* v, int length) const
	{
		(*this)[index]->type = metaffi_bool_array_type;
		(*this)[index]->cdt_val.metaffi_bool_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_bool_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_bool_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_bool_array_val.vals = (metaffi_bool*)malloc(sizeof(metaffi_bool)*length);
		for(int i=0 ; i<length ; i++){
			(*this)[index]->cdt_val.metaffi_bool_array_val.vals[i] = v[i] ? 1 : 0;
		}
	}
	
	void cdts_wrapper::set(int index, const std::vector<std::string>& v) const // string8[]
	{
		(*this)[index]->type = metaffi_string8_array_type;
		(*this)[index]->cdt_val.metaffi_string8_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_string8_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_string8_array_val.dimensions_lengths[0] = v.size();
		(*this)[index]->cdt_val.metaffi_string8_array_val.vals = (metaffi_string8*)malloc(sizeof(metaffi_string8*)*v.size());
		(*this)[index]->cdt_val.metaffi_string8_array_val.vals_sizes = (metaffi_size*)malloc(sizeof(metaffi_size*)*v.size());
		for(int i=0 ; i<v.size() ; i++)
		{
			((char**)(*this)[index]->cdt_val.metaffi_string8_array_val.vals)[i] = (char*)calloc(1, v[i].size()+1);
			std::copy(v[i].begin(), v[i].end(), ((char**)(*this)[index]->cdt_val.metaffi_string8_array_val.vals)[i]);
			((metaffi_size*)(*this)[index]->cdt_val.metaffi_string8_array_val.vals_sizes)[i] = v[i].length();
		}
	}
	
	void cdts_wrapper::set(int index, cdt_metaffi_handle* v, int length) const
	{
		(*this)[index]->type = metaffi_handle_array_type;
		(*this)[index]->cdt_val.metaffi_handle_array_val.dimensions = 1;
		(*this)[index]->cdt_val.metaffi_handle_array_val.dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		(*this)[index]->cdt_val.metaffi_handle_array_val.dimensions_lengths[0] = length;
		(*this)[index]->cdt_val.metaffi_handle_array_val.vals = (struct cdt_metaffi_handle*)malloc(sizeof(struct cdt_metaffi_handle)*length);
		for(int i=0 ; i<length ; i++){
			((*this)[index]->cdt_val.metaffi_handle_array_val.vals)[i] = v[i];
		}
	}

	void cdts_wrapper::set(int index, metaffi_callable v, const std::vector<metaffi_type>& parameters_types, const std::vector<metaffi_type>& retvals_types) const
	{
		cdt* c = (*this)[index];

		c->type = metaffi_callable_type;
		c->cdt_val.metaffi_callable_val.val = v;
		c->cdt_val.metaffi_callable_val.params_types_length = parameters_types.size();
		c->cdt_val.metaffi_callable_val.retval_types_length = retvals_types.size();
		c->cdt_val.metaffi_callable_val.parameters_types = (metaffi_type*)malloc(sizeof(metaffi_type)*parameters_types.size());
		c->cdt_val.metaffi_callable_val.retval_types = (metaffi_type*)malloc(sizeof(metaffi_type)*retvals_types.size());;

		for(int i=0 ; i<c->cdt_val.metaffi_callable_val.params_types_length ; i++){
			c->cdt_val.metaffi_callable_val.parameters_types[i] = parameters_types[i];
		}

		for(int i=0 ; i<c->cdt_val.metaffi_callable_val.retval_types_length ; i++){
			c->cdt_val.metaffi_callable_val.retval_types[i] = retvals_types[i];
		}
	}
	
	metaffi_float64 cdts_wrapper::get_metaffi_float64(int index) const
	{
		if((*this)[index]->type != metaffi_float64_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_float64, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_float64_val.val;
	}
	
	metaffi_float32 cdts_wrapper::get_metaffi_float32(int index) const
	{
		if((*this)[index]->type != metaffi_float32_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_float32, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_float32_val.val;
	}
	
	metaffi_int8 cdts_wrapper::get_metaffi_int8(int index) const
	{
		if((*this)[index]->type != metaffi_int8_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int8, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_int8_val.val;
	}
	
	metaffi_int16 cdts_wrapper::get_metaffi_int16(int index) const
	{
		if((*this)[index]->type != metaffi_int16_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int16, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_int16_val.val;
	}
	
	metaffi_int32 cdts_wrapper::get_metaffi_int32(int index) const
	{
		if((*this)[index]->type != metaffi_int32_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int32, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_int32_val.val;
	}
	
	metaffi_int64 cdts_wrapper::get_metaffi_int64(int index) const
	{
		if((*this)[index]->type != metaffi_int64_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int64, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_int64_val.val;
	}
	
	metaffi_uint8 cdts_wrapper::get_metaffi_uint8(int index) const
	{
		if((*this)[index]->type != metaffi_uint8_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint8, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_uint8_val.val;
	}
	
	metaffi_uint16 cdts_wrapper::get_metaffi_uint16(int index) const
	{
		if((*this)[index]->type != metaffi_uint16_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint16, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_uint16_val.val;
	}
	
	metaffi_uint32 cdts_wrapper::get_metaffi_uint32(int index) const
	{
		if((*this)[index]->type != metaffi_uint32_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint32, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_uint32_val.val;
	}
	
	metaffi_uint64 cdts_wrapper::get_metaffi_uint64(int index) const
	{
		if((*this)[index]->type != metaffi_uint64_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint64, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_uint64_val.val;
	}
	
	bool cdts_wrapper::get_bool(int index) const
	{
		if((*this)[index]->type != metaffi_bool_type)
		{
			std::stringstream ss;
			ss << "Requested bool, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_bool_val.val;
	}
	
	std::string cdts_wrapper::get_string(int index) const
	{
		if((*this)[index]->type != metaffi_string8_type)
		{
			std::stringstream ss;
			ss << "Requested string, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_string8_val.val;
	}
	
	metaffi_handle cdts_wrapper::get_metaffi_handle(int index) const
	{
		if((*this)[index]->type != metaffi_handle_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_handle, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		return (*this)[index]->cdt_val.metaffi_handle_val.val;
	}
	
	std::vector<metaffi_float64> cdts_wrapper::get_metaffi_float64_array(int index) const
	{
		if((*this)[index]->type != metaffi_float64_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_float64_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_float64> res;
		res.reserve((*this)[index]->cdt_val.metaffi_float64_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_float64_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_float64_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_float32> cdts_wrapper::get_metaffi_float32_array(int index) const
	{
		if((*this)[index]->type != metaffi_float32_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_float32_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_float32> res;
		res.reserve((*this)[index]->cdt_val.metaffi_float32_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_float32_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_float32_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_int8> cdts_wrapper::get_metaffi_int8_array(int index) const
	{
		if((*this)[index]->type != metaffi_int8_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int8_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_int8> res;
		res.reserve((*this)[index]->cdt_val.metaffi_int8_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_int8_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_int8_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_int16> cdts_wrapper::get_metaffi_int16_array(int index) const
	{
		if((*this)[index]->type != metaffi_int16_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int16_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_int16> res;
		res.reserve((*this)[index]->cdt_val.metaffi_int16_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_int16_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_int16_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_int32> cdts_wrapper::get_metaffi_int32_array(int index) const
	{
		if((*this)[index]->type != metaffi_int32_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int32_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_int32> res;
		res.reserve((*this)[index]->cdt_val.metaffi_int32_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_int32_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_int32_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_int64> cdts_wrapper::get_metaffi_int64_array(int index) const
	{
		if((*this)[index]->type != metaffi_int64_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_int64_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_int64> res;
		res.reserve((*this)[index]->cdt_val.metaffi_int64_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_int64_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_int64_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_uint8> cdts_wrapper::get_metaffi_uint8_array(int index) const
	{
		if((*this)[index]->type != metaffi_uint8_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint8_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_uint8> res;
		res.reserve((*this)[index]->cdt_val.metaffi_uint8_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_uint8_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_uint8_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_uint16> cdts_wrapper::get_metaffi_uint16_array(int index) const
	{
		if((*this)[index]->type != metaffi_uint16_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint16_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_uint16> res;
		res.reserve((*this)[index]->cdt_val.metaffi_uint16_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_uint16_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_uint16_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_uint32> cdts_wrapper::get_metaffi_uint32_array(int index) const
	{
		if((*this)[index]->type != metaffi_uint32_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint32_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_uint32> res;
		res.reserve((*this)[index]->cdt_val.metaffi_uint32_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_uint32_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_uint32_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<metaffi_uint64> cdts_wrapper::get_metaffi_uint64_array(int index) const
	{
		if((*this)[index]->type != metaffi_uint64_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_uint64_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<metaffi_uint64> res;
		res.reserve((*this)[index]->cdt_val.metaffi_uint64_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_uint64_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_uint64_array_val.vals[i]);
		}
		
		return res;
	}
	
	std::vector<bool> cdts_wrapper::get_bool_array(int index) const
	{
		if((*this)[index]->type != metaffi_bool_array_type)
		{
			std::stringstream ss;
			ss << "Requested bool_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<bool> res;
		res.reserve((*this)[index]->cdt_val.metaffi_bool_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_bool_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_bool_array_val.vals[i] != 0);
		}
		
		return res;
	}
	
	std::vector<std::string> cdts_wrapper::get_vector_string(int index) const
	{
		if((*this)[index]->type != metaffi_string8_type)
		{
			std::stringstream ss;
			ss << "Requested vector_string, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<std::string> res;
		res.reserve((*this)[index]->cdt_val.metaffi_string8_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_string8_array_val.dimensions_lengths[0] ; i++)
		{
			res.emplace_back((*this)[index]->cdt_val.metaffi_string8_array_val.vals[i], (*this)[index]->cdt_val.metaffi_string8_array_val.vals_sizes[i]);
		}
		
		return res;
	}
	
	std::vector<cdt_metaffi_handle> cdts_wrapper::get_metaffi_handle_array(int index) const
	{
		if((*this)[index]->type != metaffi_handle_array_type)
		{
			std::stringstream ss;
			ss << "Requested metaffi_handle_array, but the type is " << (*this)[index]->type;
			throw std::runtime_error(ss.str());
		}
		
		std::vector<cdt_metaffi_handle> res;
		res.reserve((*this)[index]->cdt_val.metaffi_handle_array_val.dimensions_lengths[0]);
		for(int i=0 ; i<(*this)[index]->cdt_val.metaffi_handle_array_val.dimensions_lengths[0] ; i++)
		{
			res.push_back((*this)[index]->cdt_val.metaffi_handle_array_val.vals[i]);
		}
		
		return res;
	}
	
	
	
	
}
