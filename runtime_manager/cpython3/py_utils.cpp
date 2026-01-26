#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_utils.h"
#include "python_api_wrapper.h"

std::string check_python_error()
{
	std::string message;

	if(!pPyErr_Occurred())
	{
		return message;
	}

	PyObject *excType, *excValue, *excTraceback = nullptr;
	pPyErr_Fetch(&excType, &excValue, &excTraceback);
	pPyErr_NormalizeException(&excType, &excValue, &excTraceback);

	// Get exception type as string
	PyObject* str_exc_type = pPyObject_Repr(excType);
	if(str_exc_type)
	{
		PyObject* pyStr_exc_type = pPyUnicode_AsEncodedString(str_exc_type, "utf-8", "strict");
		if(pyStr_exc_type)
		{
			message = PyBytes_AS_STRING(pyStr_exc_type);
			Py_DECREF(pyStr_exc_type);
		}
		Py_DECREF(str_exc_type);
	}

	// Get exception value as string
	PyObject* str_exc_value = pPyObject_Repr(excValue);
	if(str_exc_value)
	{
		PyObject* pyStr_exc_value = pPyUnicode_AsEncodedString(str_exc_value, "utf-8", "strict");
		if(pyStr_exc_value)
		{
			message += ": " + std::string(PyBytes_AS_STRING(pyStr_exc_value));
			Py_DECREF(pyStr_exc_value);
		}
		Py_DECREF(str_exc_value);
	}

	// Get traceback if available
	if(excTraceback != nullptr)
	{
		PyObject* module_name = pPyUnicode_FromString("traceback");
		if(module_name)
		{
			PyObject* pyth_module = pPyImport_Import(module_name);
			Py_DECREF(module_name);
			
			if(pyth_module)
			{
				PyObject* pyth_func = pPyObject_GetAttrString(pyth_module, "format_tb");
				if(pyth_func)
				{
					PyObject* pyth_val = pPyObject_CallFunctionObjArgs(pyth_func, excTraceback, NULL);
					if(pyth_val)
					{
						PyObject* separator = pPyUnicode_FromString("");
						if(separator)
						{
							PyObject* pyth_str = pPyUnicode_Join(separator, pyth_val);
							Py_DECREF(separator);
							
							if(pyth_str)
							{
								PyObject* pyStr = pPyUnicode_AsEncodedString(pyth_str, "utf-8", "strict");
								if(pyStr)
								{
									message += "\n";
									message += PyBytes_AS_STRING(pyStr);
									Py_DECREF(pyStr);
								}
								Py_DECREF(pyth_str);
							}
						}
						Py_DECREF(pyth_val);
					}
					Py_DECREF(pyth_func);
				}
				Py_DECREF(pyth_module);
			}
		}
	}

	Py_XDECREF(excType);
	Py_XDECREF(excValue);
	Py_XDECREF(excTraceback);

	return message;
}
