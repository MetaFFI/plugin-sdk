#pragma once

#ifdef __cplusplus
#include <string>
#include <vector>
#else
#include <string.h>
#endif

#include "python_h_declares.h"

// Function pointer types for Python API functions
typedef PyThreadState* (*PyEval_SaveThread_t)(void);
typedef void (*PyEval_RestoreThread_t)(PyThreadState *tstate);
typedef PyGILState_STATE (*pPyGILState_Ensure_t)(void);
typedef void (*PyGILState_Release_t)(PyGILState_STATE);
typedef PyThreadState* (*PyGILState_GetThisThreadState_t)(void);
typedef int (*PyGILState_Check_t)(void);

typedef void (*Py_Initialize_t)(void);
typedef void (*Py_InitializeEx_t)(int initsigs);
typedef void (*Py_Finalize_t)(void);
typedef int (*Py_FinalizeEx_t)(void);
typedef int (*Py_IsInitialized_t)(void);

typedef PyObject* (*PyUnicode_FromString_t)(const char *u);
typedef PyObject* (*PyUnicode_FromStringAndSize_t)(const char *u, Py_ssize_t size);
typedef PyObject* (*PyUnicode_FromFormat_t)(const char *format, ...);
typedef const char* (*PyUnicode_AsUTF8_t)(PyObject *unicode);
typedef const char* (*PyUnicode_AsUTF8AndSize_t)(PyObject *unicode, Py_ssize_t *size);

typedef PyObject* (*PyLong_FromLong_t)(long);
typedef PyObject* (*PyLong_FromUnsignedLong_t)(unsigned long);
typedef PyObject* (*PyLong_FromDouble_t)(double);
typedef PyObject* (*PyLong_FromString_t)(const char *str, char **pend, int base);
typedef PyObject* (*PyLong_FromLongLong_t)(long long);
typedef PyObject* (*PyLong_FromUnsignedLongLong_t)(unsigned long long);
typedef long (*PyLong_AsLong_t)(PyObject *);
typedef long long (*PyLong_AsLongLong_t)(PyObject *);
typedef unsigned long (*PyLong_AsUnsignedLong_t)(PyObject *);
typedef unsigned long long (*PyLong_AsUnsignedLongLong_t)(PyObject *);

typedef PyObject* (*PyFloat_FromDouble_t)(double);
typedef PyObject* (*PyFloat_FromString_t)(PyObject*);
typedef double (*PyFloat_AsDouble_t)(PyObject *);

typedef PyObject* (*PyList_New_t)(Py_ssize_t size);
typedef Py_ssize_t (*PyList_Size_t)(PyObject *);
typedef PyObject* (*PyList_GetItem_t)(PyObject *, Py_ssize_t);
typedef int (*PyList_SetItem_t)(PyObject *, Py_ssize_t, PyObject *);
typedef int (*PyList_Insert_t)(PyObject *, Py_ssize_t, PyObject *);
typedef int (*PyList_Append_t)(PyObject *, PyObject *);
typedef PyObject* (*PyList_GetSlice_t)(PyObject *, Py_ssize_t, Py_ssize_t);
typedef int (*PyList_SetSlice_t)(PyObject *, Py_ssize_t, Py_ssize_t, PyObject *);

typedef PyObject* (*PyTuple_New_t)(Py_ssize_t size);
typedef Py_ssize_t (*PyTuple_Size_t)(PyObject *);
typedef PyObject* (*PyTuple_GetItem_t)(PyObject *, Py_ssize_t);
typedef int (*PyTuple_SetItem_t)(PyObject *, Py_ssize_t, PyObject *);
typedef PyObject* (*PyTuple_GetSlice_t)(PyObject *, Py_ssize_t, Py_ssize_t);

typedef PyObject* (*PyDict_New_t)(void);
typedef int (*PyDict_SetItem_t)(PyObject *mp, PyObject *key, PyObject *item);
typedef int (*PyDict_SetItemString_t)(PyObject *dp, const char *key, PyObject *item);
typedef PyObject* (*PyDict_GetItem_t)(PyObject *mp, PyObject *key);
typedef PyObject* (*PyDict_GetItemString_t)(PyObject *dp, const char *key);
typedef int (*PyDict_DelItem_t)(PyObject *mp, PyObject *key);
typedef int (*PyDict_DelItemString_t)(PyObject *dp, const char *key);
typedef int (*PyDict_Clear_t)(PyObject *mp);
typedef int (*PyDict_Next_t)(PyObject *mp, Py_ssize_t *pos, PyObject **key, PyObject **value);
typedef Py_ssize_t (*PyDict_Size_t)(PyObject *mp);

typedef void (*PyErr_SetString_t)(PyObject *type, const char *message);
typedef void (*PyErr_SetObject_t)(PyObject *type, PyObject *value);
typedef PyObject* (*PyErr_Occurred_t)(void);
typedef void (*PyErr_Clear_t)(void);
typedef void (*PyErr_Print_t)(void);
typedef void (*PyErr_WriteUnraisable_t)(PyObject *obj);
typedef int (*PyErr_ExceptionMatches_t)(PyObject *exc);
typedef int (*PyErr_GivenExceptionMatches_t)(PyObject *given, PyObject *exc);
typedef void (*PyErr_Fetch_t)(PyObject **ptype, PyObject **pvalue, PyObject **ptraceback);
typedef void (*PyErr_Restore_t)(PyObject *type, PyObject *value, PyObject *traceback);

typedef int (*PyObject_Print_t)(PyObject *o, FILE *fp, int flags);
typedef int (*PyObject_HasAttrString_t)(PyObject *o, const char *attr_name);
typedef PyObject* (*PyObject_GetAttrString_t)(PyObject *o, const char *attr_name);
typedef int (*PyObject_HasAttr_t)(PyObject *o, PyObject *attr_name);
typedef PyObject* (*PyObject_GetAttr_t)(PyObject *o, PyObject *attr_name);
typedef int (*PyObject_SetAttrString_t)(PyObject *o, const char *attr_name, PyObject *v);
typedef int (*PyObject_SetAttr_t)(PyObject *o, PyObject *attr_name, PyObject *v);
typedef PyObject* (*PyObject_CallObject_t)(PyObject *callable, PyObject *args);
typedef PyObject* (*PyObject_Call_t)(PyObject *callable, PyObject *args, PyObject *kwargs);
typedef PyObject* (*PyObject_CallNoArgs_t)(PyObject *func);
typedef PyObject* (*PyObject_CallFunction_t)(PyObject *callable, const char *format, ...);
typedef PyObject* (*PyObject_CallMethod_t)(PyObject *obj, const char *name, const char *format, ...);
typedef PyObject* (*PyObject_Type_t)(PyObject *o);
typedef Py_ssize_t (*PyObject_Size_t)(PyObject *o);
typedef PyObject* (*PyObject_GetItem_t)(PyObject *o, PyObject *key);
typedef int (*PyObject_SetItem_t)(PyObject *o, PyObject *key, PyObject *v);
typedef int (*PyObject_DelItem_t)(PyObject *o, PyObject *key);
typedef int (*PyObject_DelItemString_t)(PyObject *o, const char *key);
typedef int (*PyObject_AsCharBuffer_t)(PyObject *obj, const char **buffer, Py_ssize_t *buffer_len);
typedef int (*PyObject_CheckReadBuffer_t)(PyObject *obj);
typedef PyObject* (*PyObject_Format_t)(PyObject* obj, PyObject* format_spec);
typedef PyObject* (*PyObject_GetIter_t)(PyObject *o);
typedef int (*PyObject_IsTrue_t)(PyObject *o);
typedef int (*PyObject_Not_t)(PyObject *o);
typedef int (*PyCallable_Check_t)(PyObject *o);
typedef PyInterpreterState* (*PyInterpreterState_Get_t)();

typedef int (*PySequence_Check_t)(PyObject *o);
typedef Py_ssize_t (*PySequence_Size_t)(PyObject *o);
typedef PyObject* (*PySequence_Concat_t)(PyObject *o1, PyObject *o2);
typedef PyObject* (*PySequence_Repeat_t)(PyObject *o, Py_ssize_t count);
typedef PyObject* (*PySequence_GetItem_t)(PyObject *o, Py_ssize_t i);
typedef PyObject* (*PySequence_GetSlice_t)(PyObject *o, Py_ssize_t i1, Py_ssize_t i2);
typedef int (*PySequence_SetItem_t)(PyObject *o, Py_ssize_t i, PyObject *v);
typedef int (*PySequence_DelItem_t)(PyObject *o, Py_ssize_t i);
typedef int (*PySequence_SetSlice_t)(PyObject *o, Py_ssize_t i1, Py_ssize_t i2, PyObject *v);
typedef int (*PySequence_DelSlice_t)(PyObject *o, Py_ssize_t i1, Py_ssize_t i2);
typedef PyObject* (*PySequence_Tuple_t)(PyObject *o);
typedef PyObject* (*PySequence_List_t)(PyObject *o);
typedef PyObject* (*PySequence_Fast_t)(PyObject *o, const char* m);
typedef Py_ssize_t (*PySequence_Count_t)(PyObject *o, PyObject *value);
typedef int (*PySequence_Contains_t)(PyObject *seq, PyObject *ob);
typedef Py_ssize_t (*PySequence_Index_t)(PyObject *o, PyObject *value);
typedef PyObject* (*PySequence_InPlaceConcat_t)(PyObject *o1, PyObject *o2);
typedef PyObject* (*PySequence_InPlaceRepeat_t)(PyObject *o, Py_ssize_t count);

typedef PyObject* (*PyImport_ImportModule_t)(const char *name);
typedef PyObject* (*PyImport_Import_t)(PyObject *name);
typedef PyObject* (*PyImport_ReloadModule_t)(PyObject *m);
typedef PyObject* (*PyImport_AddModule_t)(const char *name);
typedef PyObject* (*PyImport_GetModuleDict_t)(void);
typedef PyObject* (*PyImport_ImportModuleLevel_t)(const char *name, PyObject *globals, PyObject *locals, PyObject *fromlist, int level);

typedef void (*_Py_Dealloc_t)(PyObject *obj);

typedef PyObject* (*PySys_GetObject_t)(const char *);
typedef int (*PySys_SetObject_t)(const char *, PyObject *);
typedef void (*PySys_WriteStdout_t)(const char *format, ...);
typedef void (*PySys_WriteStderr_t)(const char *format, ...);
typedef void (*PySys_FormatStdout_t)(const char *format, ...);
typedef void (*PySys_FormatStderr_t)(const char *format, ...);
typedef void (*PySys_ResetWarnOptions_t)(void);
typedef PyObject* (*PySys_GetXOptions_t)(void);

typedef PyObject* (*Py_CompileString_t)(const char *, const char *, int);
typedef int (*PyRun_SimpleString_t)(const char *);
typedef void (*PyErr_PrintEx_t)(int);
typedef void (*PyErr_Display_t)(PyObject *, PyObject *, PyObject *);

typedef PyObject* (*PyBool_FromLong_t)(long);
typedef PyObject* (*PyBytes_FromString_t)(const char *);
typedef PyObject* (*PyBytes_FromStringAndSize_t)(const char *, Py_ssize_t);
typedef Py_ssize_t (*PyBytes_Size_t)(PyObject *);
typedef char* (*PyBytes_AsString_t)(PyObject *);
typedef int (*PyBytes_AsStringAndSize_t)(PyObject *, char **, Py_ssize_t *);

typedef void* (*PyLong_AsVoidPtr_t)(PyObject *);
typedef PyObject* (*PyLong_FromVoidPtr_t)(void *);

typedef PyObject* (*PyMapping_GetItemString_t)(PyObject *o, const char *key);

typedef PyObject* (*PyUnicode_DecodeUTF8_t)(const char *s, Py_ssize_t size, const char *errors);
typedef PyObject* (*PyUnicode_AsEncodedString_t)(PyObject *unicode, const char *encoding, const char *errors);
typedef PyObject* (*PyUnicode_Decode_t)(const char *s, Py_ssize_t size, const char *encoding, const char *errors);
typedef PyObject* (*PyUnicode_FromEncodedObject_t)(PyObject *obj, const char *encoding, const char *errors);
typedef Py_ssize_t (*PyUnicode_GetLength_t)(PyObject *unicode);
typedef PyObject* (*PyUnicode_FromKindAndData_t)(int kind, const void *buffer, Py_ssize_t size);
typedef PyObject* (*PyUnicode_AsUTF16String_t)(PyObject *unicode);
typedef PyObject* (*PyUnicode_AsUTF32String_t)(PyObject *unicode);

typedef PyObject* (*PyModule_GetDict_t)(PyObject *module);
typedef PyObject* (*PyUnicode_AsUTF8String_t)(PyObject *unicode);

typedef void (*PyErr_NormalizeException_t)(PyObject**, PyObject**, PyObject**);
typedef PyObject* (*PyObject_Repr_t)(PyObject *);

typedef PyObject* (*PyObject_CallFunctionObjArgs_t)(PyObject *callable, ...);
typedef PyObject* (*PyUnicode_Join_t)(PyObject *separator, PyObject *seq);
typedef int (*PyObject_RichCompareBool_t)(PyObject *o1, PyObject *o2, int opid);
typedef int (*PyObject_IsInstance_t)(PyObject *inst, PyObject *cls);

// Extern declarations of function pointers
extern PyEval_SaveThread_t pPyEval_SaveThread;
extern PyEval_RestoreThread_t pPyEval_RestoreThread;
extern pPyGILState_Ensure_t pPyGILState_Ensure;
extern PyGILState_Release_t pPyGILState_Release;
extern PyGILState_GetThisThreadState_t pPyGILState_GetThisThreadState;
extern PyGILState_Check_t pPyGILState_Check;

extern Py_Initialize_t pPy_Initialize;
extern Py_InitializeEx_t pPy_InitializeEx;
extern Py_Finalize_t pPy_Finalize;
extern Py_FinalizeEx_t pPy_FinalizeEx;
extern Py_IsInitialized_t pPy_IsInitialized;

extern PyUnicode_FromString_t pPyUnicode_FromString;
extern PyUnicode_FromStringAndSize_t pPyUnicode_FromStringAndSize;
extern PyUnicode_FromFormat_t pPyUnicode_FromFormat;
extern PyUnicode_AsUTF8_t pPyUnicode_AsUTF8;
extern PyUnicode_AsUTF8AndSize_t pPyUnicode_AsUTF8AndSize;

extern PyLong_FromLong_t pPyLong_FromLong;
extern PyLong_FromUnsignedLong_t pPyLong_FromUnsignedLong;
extern PyLong_FromDouble_t pPyLong_FromDouble;
extern PyLong_FromString_t pPyLong_FromString;
extern PyLong_FromLongLong_t pPyLong_FromLongLong;
extern PyLong_FromUnsignedLongLong_t pPyLong_FromUnsignedLongLong;
extern PyLong_AsLong_t pPyLong_AsLong;
extern PyLong_AsLongLong_t pPyLong_AsLongLong;
extern PyLong_AsUnsignedLong_t pPyLong_AsUnsignedLong;
extern PyLong_AsUnsignedLongLong_t pPyLong_AsUnsignedLongLong;

extern PyFloat_FromDouble_t pPyFloat_FromDouble;
extern PyFloat_FromString_t pPyFloat_FromString;
extern PyFloat_AsDouble_t pPyFloat_AsDouble;

extern PyList_New_t pPyList_New;
extern PyList_Size_t pPyList_Size;
extern PyList_GetItem_t pPyList_GetItem;
extern PyList_SetItem_t pPyList_SetItem;
extern PyList_Insert_t pPyList_Insert;
extern PyList_Append_t pPyList_Append;
extern PyList_GetSlice_t pPyList_GetSlice;
extern PyList_SetSlice_t pPyList_SetSlice;

extern PyTuple_New_t pPyTuple_New;
extern PyTuple_Size_t pPyTuple_Size;
extern PyTuple_GetItem_t pPyTuple_GetItem;
extern PyTuple_SetItem_t pPyTuple_SetItem;
extern PyTuple_GetSlice_t pPyTuple_GetSlice;

extern PyDict_New_t pPyDict_New;
extern PyDict_SetItem_t pPyDict_SetItem;
extern PyDict_SetItemString_t pPyDict_SetItemString;
extern PyDict_GetItem_t pPyDict_GetItem;
extern PyDict_GetItemString_t pPyDict_GetItemString;
extern PyDict_DelItem_t pPyDict_DelItem;
extern PyDict_DelItemString_t pPyDict_DelItemString;
extern PyDict_Clear_t pPyDict_Clear;
extern PyDict_Next_t pPyDict_Next;
extern PyDict_Size_t pPyDict_Size;

extern PyErr_SetString_t pPyErr_SetString;
extern PyErr_SetObject_t pPyErr_SetObject;
extern PyErr_Occurred_t pPyErr_Occurred;
extern PyErr_Clear_t pPyErr_Clear;
extern PyErr_Print_t pPyErr_Print;
extern PyErr_WriteUnraisable_t pPyErr_WriteUnraisable;
extern PyErr_ExceptionMatches_t pPyErr_ExceptionMatches;
extern PyErr_GivenExceptionMatches_t pPyErr_GivenExceptionMatches;
extern PyErr_Fetch_t pPyErr_Fetch;
extern PyErr_Restore_t pPyErr_Restore;

extern PyObject_Print_t pPyObject_Print;
extern PyObject_HasAttrString_t pPyObject_HasAttrString;
extern PyObject_GetAttrString_t pPyObject_GetAttrString;
extern PyObject_HasAttr_t pPyObject_HasAttr;
extern PyObject_GetAttr_t pPyObject_GetAttr;
extern PyObject_SetAttrString_t pPyObject_SetAttrString;
extern PyObject_SetAttr_t pPyObject_SetAttr;
extern PyObject_CallObject_t pPyObject_CallObject;
extern PyObject_Call_t pPyObject_Call;
extern PyObject_CallNoArgs_t pPyObject_CallNoArgs;
extern PyObject_CallFunction_t pPyObject_CallFunction;
extern PyObject_CallMethod_t pPyObject_CallMethod;
extern PyObject_Type_t pPyObject_Type;
extern PyObject_Size_t pPyObject_Size;
extern PyObject_GetItem_t pPyObject_GetItem;
extern PyObject_SetItem_t pPyObject_SetItem;
extern PyObject_DelItem_t pPyObject_DelItem;
extern PyObject_DelItemString_t pPyObject_DelItemString;
extern PyObject_AsCharBuffer_t pPyObject_AsCharBuffer;
extern PyObject_CheckReadBuffer_t pPyObject_CheckReadBuffer;
extern PyObject_Format_t pPyObject_Format;
extern PyObject_GetIter_t pPyObject_GetIter;
extern PyObject_IsTrue_t pPyObject_IsTrue;
extern PyObject_Not_t pPyObject_Not;
extern PyCallable_Check_t pPyCallable_Check;
extern PyInterpreterState_Get_t pPyInterpreterState_Get;

extern PySequence_Check_t pPySequence_Check;
extern PySequence_Size_t pPySequence_Size;
extern PySequence_Concat_t pPySequence_Concat;
extern PySequence_Repeat_t pPySequence_Repeat;
extern PySequence_GetItem_t pPySequence_GetItem;
extern PySequence_GetSlice_t pPySequence_GetSlice;
extern PySequence_SetItem_t pPySequence_SetItem;
extern PySequence_DelItem_t pPySequence_DelItem;
extern PySequence_SetSlice_t pPySequence_SetSlice;
extern PySequence_DelSlice_t pPySequence_DelSlice;
extern PySequence_Tuple_t pPySequence_Tuple;
extern PySequence_List_t pPySequence_List;
extern PySequence_Fast_t pPySequence_Fast;
extern PySequence_Count_t pPySequence_Count;
extern PySequence_Contains_t pPySequence_Contains;
extern PySequence_Index_t pPySequence_Index;
extern PySequence_InPlaceConcat_t pPySequence_InPlaceConcat;
extern PySequence_InPlaceRepeat_t pPySequence_InPlaceRepeat;

extern PyImport_ImportModule_t pPyImport_ImportModule;
extern PyImport_Import_t pPyImport_Import;
extern PyImport_ReloadModule_t pPyImport_ReloadModule;
extern PyImport_AddModule_t pPyImport_AddModule;
extern PyImport_GetModuleDict_t pPyImport_GetModuleDict;
extern PyImport_ImportModuleLevel_t pPyImport_ImportModuleLevel;

extern _Py_Dealloc_t p_Py_Dealloc;

extern PySys_GetObject_t pPySys_GetObject;
extern PySys_SetObject_t pPySys_SetObject;
extern PySys_WriteStdout_t pPySys_WriteStdout;
extern PySys_WriteStderr_t pPySys_WriteStderr;
extern PySys_FormatStdout_t pPySys_FormatStdout;
extern PySys_FormatStderr_t pPySys_FormatStderr;
extern PySys_ResetWarnOptions_t pPySys_ResetWarnOptions;
extern PySys_GetXOptions_t pPySys_GetXOptions;

extern Py_CompileString_t pPy_CompileString;
extern PyRun_SimpleString_t pPyRun_SimpleString;
extern PyErr_PrintEx_t pPyErr_PrintEx;
extern PyErr_Display_t pPyErr_Display;

extern PyBool_FromLong_t pPyBool_FromLong;
extern PyBytes_FromString_t pPyBytes_FromString;
extern PyBytes_FromStringAndSize_t pPyBytes_FromStringAndSize;
extern PyBytes_Size_t pPyBytes_Size;
extern PyBytes_AsString_t pPyBytes_AsString;
extern PyBytes_AsStringAndSize_t pPyBytes_AsStringAndSize;

extern PyLong_AsVoidPtr_t pPyLong_AsVoidPtr;
extern PyLong_FromVoidPtr_t pPyLong_FromVoidPtr;

extern PyMapping_GetItemString_t pPyMapping_GetItemString;

extern PyUnicode_DecodeUTF8_t pPyUnicode_DecodeUTF8;
extern PyUnicode_AsEncodedString_t pPyUnicode_AsEncodedString;
extern PyUnicode_Decode_t pPyUnicode_Decode;
extern PyUnicode_FromEncodedObject_t pPyUnicode_FromEncodedObject;
extern PyUnicode_GetLength_t pPyUnicode_GetLength;
extern PyUnicode_FromKindAndData_t pPyUnicode_FromKindAndData;
extern PyUnicode_AsUTF16String_t pPyUnicode_AsUTF16String;
extern PyUnicode_AsUTF32String_t pPyUnicode_AsUTF32String;

extern PyModule_GetDict_t pPyModule_GetDict;
extern PyUnicode_AsUTF8String_t pPyUnicode_AsUTF8String;

extern PyErr_NormalizeException_t pPyErr_NormalizeException;
extern PyObject_Repr_t pPyObject_Repr;

extern PyObject_CallFunctionObjArgs_t pPyObject_CallFunctionObjArgs;
extern PyUnicode_Join_t pPyUnicode_Join;
extern PyObject_RichCompareBool_t pPyObject_RichCompareBool;
extern PyObject_IsInstance_t pPyObject_IsInstance;

// Type pointers
extern PyObject* pPyType_Type;
extern PyObject* pPyBaseObject_Type;
extern PyObject* pPySuper_Type;
extern PyObject* pPyBool_Type;
extern PyObject* pPyFloat_Type;
extern PyObject* pPyLong_Type;
extern PyObject* pPyList_Type;
extern PyObject* pPyDict_Type;
extern PyObject* pPyUnicode_Type;
extern PyObject* pPyBytes_Type;
extern PyObject* pPyExc_RuntimeError;
extern PyObject* pPyExc_ValueError;
extern PyObject* pPyProperty_Type;
extern PyObject* pPyTuple_Type;

// Macros
#define PyBytes_AS_STRING(op) (pPyBytes_AsString(op))
#define Py_IsNone(x) ((x) == pPy_None)

#ifdef __cplusplus
// Function to detect installed Python 3 versions (3.8-3.13)
// Returns vector of version strings like "3.8", "3.9", etc.
std::vector<std::string> detect_installed_python3();

// Function to load Python API for a specific version
// version should be like "3.8", "3.9", etc.
bool load_python3_api(const std::string& version);

#ifndef _WIN32
void load_python3_variables_from_interpreter();
#endif
#endif
