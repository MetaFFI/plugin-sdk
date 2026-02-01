#include "python_utils.h"

namespace metaffi::runtime::cpython3
{

std::string check_python_error()
{
	if(!pPyErr_Occurred())
	{
		return "";
	}

	PyObject* ptype = nullptr;
	PyObject* pvalue = nullptr;
	PyObject* ptraceback = nullptr;

	pPyErr_Fetch(&ptype, &pvalue, &ptraceback);

	std::string error_msg;
	if(pvalue)
	{
		PyObject* pstr = pPyObject_Str(pvalue);
		if(pstr)
		{
			const char* err_str = pPyUnicode_AsUTF8(pstr);
			if(err_str)
			{
				error_msg = err_str;
			}
			Py_DECREF(pstr);
		}
		Py_DECREF(pvalue);
	}

	if(ptype)
	{
		Py_DECREF(ptype);
	}
	if(ptraceback)
	{
		Py_DECREF(ptraceback);
	}

	return error_msg;
}

}
