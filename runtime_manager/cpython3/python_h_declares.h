#pragma once

// Python version info - will be set dynamically based on loaded version
// This file provides forward declarations and type definitions

// Rich comparison opcodes
#define Py_LT 0
#define Py_LE 1
#define Py_EQ 2
#define Py_NE 3
#define Py_GT 4
#define Py_GE 5

// PyUnicode_Kind enum
typedef enum PyUnicode_Kind {
    PyUnicode_WCHAR_KIND = 0,
    PyUnicode_1BYTE_KIND = 1,
    PyUnicode_2BYTE_KIND = 2,
    PyUnicode_4BYTE_KIND = 4
} PyUnicode_Kind;

// Basic types
#ifdef _WIN64
    typedef long long Py_ssize_t;
    typedef long long Py_hash_t;
#else
    typedef long Py_ssize_t;
    typedef long Py_hash_t;
#endif

// Basic object structure
struct _object {
    Py_ssize_t ob_refcnt;
    struct _typeobject *ob_type;
};
typedef struct _object PyObject;

// Variable-size object structure
typedef struct {
    PyObject ob_base;
    Py_ssize_t ob_size;
} PyVarObject;

// Object header macros
#define PyObject_HEAD  struct _object ob_base;
#define PyObject_VAR_HEAD  struct { PyObject ob_base; Py_ssize_t ob_size; } ob_base;

#define PyImport_ImportModuleEx(n, g, l, f) \
    pPyImport_ImportModuleLevel(n, g, l, f, 0)

// Forward declarations of opaque types
typedef struct _typeobject PyTypeObject;
typedef struct _ts PyThreadState;
typedef struct _is PyInterpreterState;
typedef struct _co PyCodeObject;
typedef struct _frame PyFrameObject;

// Forward declaration of buffer info struct
struct bufferinfo;

// Forward declarations for struct dependencies
struct _frame;
struct _PyInterpreterFrame;
struct _err_stackitem;
struct _stack_chunk;
struct _PyCFrame;

// Code address range struct
typedef struct PyCodeAddressRange {
    int start_line;
    int end_line;
    int start_col;
    int end_col;
} PyCodeAddressRange;

// Common function pointer types used in method structs
typedef PyObject *(*unaryfunc)(PyObject *);
typedef PyObject *(*binaryfunc)(PyObject *, PyObject *);
typedef PyObject *(*ternaryfunc)(PyObject *, PyObject *, PyObject *);
typedef int (*inquiry)(PyObject *);
typedef Py_ssize_t (*lenfunc)(PyObject *);
typedef PyObject *(*ssizeargfunc)(PyObject *, Py_ssize_t);
typedef int (*ssizeobjargproc)(PyObject *, Py_ssize_t, PyObject *);
typedef int (*objobjproc)(PyObject *, PyObject *);
typedef int (*objobjargproc)(PyObject *, PyObject *, PyObject *);
typedef void (*destructor)(PyObject *);
typedef PyObject *(*getattrfunc)(PyObject *, char *);
typedef int (*setattrfunc)(PyObject *, char *, PyObject *);
typedef PyObject *(*reprfunc)(PyObject *);
typedef Py_hash_t (*hashfunc)(PyObject *);
typedef void (*freefunc)(void *);
typedef int (*getbufferproc)(PyObject *, struct bufferinfo *, int);
typedef void (*releasebufferproc)(PyObject *, struct bufferinfo *);

// Additional function pointer types not already defined
typedef int (*Py_tracefunc)(PyObject *, struct _frame *, int, PyObject *);
typedef int (*visitproc)(PyObject *, void *);
typedef int (*traverseproc)(PyObject *, visitproc, void *);
typedef PyObject* (*getattrofunc)(PyObject *, PyObject *);
typedef int (*setattrofunc)(PyObject *, PyObject *, PyObject *);
typedef PyObject* (*richcmpfunc) (PyObject *, PyObject *, int);
typedef PyObject* (*getiterfunc) (PyObject *);
typedef PyObject* (*iternextfunc) (PyObject *);
typedef PyObject* (*descrgetfunc) (PyObject *, PyObject *, PyObject *);
typedef int (*descrsetfunc) (PyObject *, PyObject *, PyObject *);
typedef int (*initproc)(PyObject *, PyObject *, PyObject *);
typedef PyObject* (*newfunc)(PyTypeObject *, PyObject *, PyObject *);
typedef PyObject* (*allocfunc)(PyTypeObject *, Py_ssize_t);
typedef PyObject* (*vectorcallfunc)(PyObject *callable, PyObject *const *args,
                                   size_t nargsf, PyObject *kwnames);

// Sequence protocol struct
typedef struct {
    lenfunc sq_length;
    binaryfunc sq_concat;
    ssizeargfunc sq_repeat;
    ssizeargfunc sq_item;
    void *was_sq_slice;
    ssizeobjargproc sq_ass_item;
    void *was_sq_ass_slice;
    objobjproc sq_contains;
    binaryfunc sq_inplace_concat;
    ssizeargfunc sq_inplace_repeat;
} PySequenceMethods;

// Mapping protocol struct
typedef struct {
    lenfunc mp_length;
    binaryfunc mp_subscript;
    objobjargproc mp_ass_subscript;
} PyMappingMethods;

// Async protocol struct
typedef struct {
    unaryfunc am_await;
    unaryfunc am_aiter;
    unaryfunc am_anext;
} PyAsyncMethods;

// Number protocol struct
typedef struct {
    binaryfunc nb_add;
    binaryfunc nb_subtract;
    binaryfunc nb_multiply;
    binaryfunc nb_remainder;
    binaryfunc nb_divmod;
    ternaryfunc nb_power;
    unaryfunc nb_negative;
    unaryfunc nb_positive;
    unaryfunc nb_absolute;
    inquiry nb_bool;
    unaryfunc nb_invert;
    binaryfunc nb_lshift;
    binaryfunc nb_rshift;
    binaryfunc nb_and;
    binaryfunc nb_xor;
    binaryfunc nb_or;
    unaryfunc nb_int;
    void *nb_reserved;
    unaryfunc nb_float;
    binaryfunc nb_inplace_add;
    binaryfunc nb_inplace_subtract;
    binaryfunc nb_inplace_multiply;
    binaryfunc nb_inplace_remainder;
    ternaryfunc nb_inplace_power;
    binaryfunc nb_inplace_lshift;
    binaryfunc nb_inplace_rshift;
    binaryfunc nb_inplace_and;
    binaryfunc nb_inplace_xor;
    binaryfunc nb_inplace_or;
    binaryfunc nb_floor_divide;
    binaryfunc nb_true_divide;
    binaryfunc nb_inplace_floor_divide;
    binaryfunc nb_inplace_true_divide;
    unaryfunc nb_index;
    binaryfunc nb_matrix_multiply;
    binaryfunc nb_inplace_matrix_multiply;
} PyNumberMethods;

// Buffer protocol struct
typedef struct {
    getbufferproc bf_getbuffer;
    releasebufferproc bf_releasebuffer;
} PyBufferProcs;

// Type object struct
struct _typeobject : public PyObject {
    PyObject_VAR_HEAD
    const char *tp_name;
    Py_ssize_t tp_basicsize;
    Py_ssize_t tp_itemsize;
    destructor tp_dealloc;
    Py_ssize_t tp_vectorcall_offset;
    getattrfunc tp_getattr;
    setattrfunc tp_setattr;
    PyAsyncMethods *tp_as_async;
    reprfunc tp_repr;
    PyNumberMethods *tp_as_number;
    PySequenceMethods *tp_as_sequence;
    PyMappingMethods *tp_as_mapping;
    hashfunc tp_hash;
    ternaryfunc tp_call;
    reprfunc tp_str;
    getattrofunc tp_getattro;
    setattrofunc tp_setattro;
    PyBufferProcs *tp_as_buffer;
    unsigned long tp_flags;
    const char *tp_doc;
    traverseproc tp_traverse;
    inquiry tp_clear;
    richcmpfunc tp_richcompare;
    Py_ssize_t tp_weaklistoffset;
    getiterfunc tp_iter;
    iternextfunc tp_iternext;
    struct PyMethodDef *tp_methods;
    struct PyMemberDef *tp_members;
    struct PyGetSetDef *tp_getset;
    struct _typeobject *tp_base;
    PyObject *tp_dict;
    descrgetfunc tp_descr_get;
    descrsetfunc tp_descr_set;
    Py_ssize_t tp_dictoffset;
    initproc tp_init;
    allocfunc tp_alloc;
    newfunc tp_new;
    freefunc tp_free;
    inquiry tp_is_gc;
    PyObject *tp_bases;
    PyObject *tp_mro;
    PyObject *tp_cache;
    PyObject *tp_subclasses;
    PyObject *tp_weaklist;
    destructor tp_del;
    unsigned int tp_version_tag;
    destructor tp_finalize;
    vectorcallfunc tp_vectorcall;
};

// PyTraceInfo struct
typedef struct {
    PyCodeObject *code;
    struct PyCodeAddressRange bounds;
} PyTraceInfo;

// PyCFrame struct
typedef struct _PyCFrame {
    uint8_t use_tracing;
    struct _PyInterpreterFrame *current_frame;
    struct _PyCFrame *previous;
} _PyCFrame;

// Error stack item struct
typedef struct _err_stackitem {
    PyObject *exc_value;
    struct _err_stackitem *previous_item;
} _PyErr_StackItem;

// Stack chunk struct
typedef struct _stack_chunk {
    struct _stack_chunk *previous;
    size_t size;
    size_t top;
    PyObject *data[1];
} _PyStackChunk;

// Thread state struct
struct _ts {
    PyThreadState *prev;
    PyThreadState *next;
    PyInterpreterState *interp;
    int _initialized;
    int _static;
    int recursion_remaining;
    int recursion_limit;
    int recursion_headroom;
    int tracing;
    int tracing_what;
    _PyCFrame *cframe;
    Py_tracefunc c_profilefunc;
    Py_tracefunc c_tracefunc;
    PyObject *c_profileobj;
    PyObject *c_traceobj;
    PyObject *curexc_type;
    PyObject *curexc_value;
    PyObject *curexc_traceback;
    _PyErr_StackItem *exc_info;
    PyObject *dict;
    int gilstate_counter;
    PyObject *async_exc;
    unsigned long thread_id;
    unsigned long native_thread_id;
    int trash_delete_nesting;
    PyObject *trash_delete_later;
    void (*on_delete)(void *);
    void *on_delete_data;
    int coroutine_origin_tracking_depth;
    PyObject *async_gen_firstiter;
    PyObject *async_gen_finalizer;
    PyObject *context;
    uint64_t context_ver;
    uint64_t id;
    PyTraceInfo trace_info;
    _PyStackChunk *datastack_chunk;
    PyObject **datastack_top;
    PyObject **datastack_limit;
    _PyErr_StackItem exc_state;
    _PyCFrame root_cframe;
};

// None object
extern PyObject* pPy_None;

// Reference counting macros
#define Py_INCREF(op) (((PyObject *)(op))->ob_refcnt++)
#define Py_DECREF(op) \
    do { \
        PyObject *_py_decref_tmp = (PyObject *)(op); \
        if (--(_py_decref_tmp)->ob_refcnt == 0) { \
            p_Py_Dealloc(_py_decref_tmp); \
        } \
    } while (0)

#define Py_XINCREF(op) do { if ((op) == NULL) ; else Py_INCREF(op); } while (0)
#define Py_XDECREF(op) do { if ((op) == NULL) ; else Py_DECREF(op); } while (0)

// Generic operations on objects
#define PyObject_GetAttr(obj, name) pPyObject_GetAttr((PyObject *)(obj), (PyObject *)(name))
#define PyObject_SetAttr(obj, name, value) pPyObject_SetAttr((PyObject *)(obj), (PyObject *)(name), (PyObject *)(value))
#define PyObject_CallObject(callable, args) pPyObject_CallObject((PyObject *)(callable), (PyObject *)(args))
#define PySequence_Contains(seq, obj) pPySequence_Contains((PyObject *)(seq), (PyObject *)(obj))

// GIL state
typedef enum PyGILState_STATE {
    PyGILState_LOCKED,
    PyGILState_UNLOCKED
} PyGILState_STATE;

// Python type flags
#define Py_TPFLAGS_DEFAULT 0
#define Py_TPFLAGS_BASETYPE (1UL << 10)
#define Py_TPFLAGS_HAVE_GC (1UL << 14)
#define Py_TPFLAGS_HEAPTYPE (1UL << 9)

// Common Python types
#define pPyBool_Check(op) (Py_TYPE(op) == pPyBool_Type)
#define pPyFloat_Check(op) (Py_TYPE(op) == pPyFloat_Type)
#define pPyLong_Check(op) (Py_TYPE(op) == pPyLong_Type)
#define pPyTuple_Check(op) (Py_TYPE(op) == pPyTuple_Type)
#define pPyList_Check(op) (Py_TYPE(op) == pPyList_Type)
#define pPyDict_Check(op) (Py_TYPE(op) == pPyDict_Type)
#define pPyUnicode_Check(op) (Py_TYPE(op) == pPyUnicode_Type)
#define pPyBytes_Check(op) (Py_TYPE(op) == pPyBytes_Type)

// Type access macros
static inline PyTypeObject* Py_TYPE(PyObject *ob) {
    return ob->ob_type;
}

static inline void Py_SET_TYPE(PyObject *ob, PyTypeObject *type) {
    ob->ob_type = type;
}

// Reference count access macros
static inline Py_ssize_t Py_REFCNT(PyObject *ob) {
    return ob->ob_refcnt;
}

static inline void Py_SET_REFCNT(PyObject *ob, Py_ssize_t refcnt) {
    ob->ob_refcnt = refcnt;
}

// Size access macros
static inline Py_ssize_t Py_SIZE(PyVarObject *ob) {
    return ob->ob_size;
}

static inline void Py_SET_SIZE(PyVarObject *ob, Py_ssize_t size) {
    ob->ob_size = size;
}

// Cast macros
#define _PyObject_CAST(op) ((PyObject*)(op))
#define _PyVarObject_CAST(op) ((PyVarObject*)(op))
