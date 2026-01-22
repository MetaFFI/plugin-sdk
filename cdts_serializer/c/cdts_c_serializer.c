#include "cdts_c_serializer.h"
#include <runtime/xllr_capi_loader.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

// Internal array context for nested arrays
typedef struct array_context {
    struct cdts* array_cdts;      // The cdts for this array level
    metaffi_size current_index;   // Current position in this array
    metaffi_size length;          // Total length of this array
    metaffi_type element_type;    // Element type (for validation)
    struct array_context* next;    // Next level (for nesting)
} array_context_t;

// Internal serializer structure
struct cdts_serializer {
    struct cdts* root_cdts;       // Root CDTS
    metaffi_size current_index;   // Current index in root
    array_context_t* array_stack; // Stack of nested arrays (NULL when not in array)
};

// Helper function to set error string
static void set_error(char** out_err, const char* format, ...) {
    if (out_err && *out_err == NULL) {
        va_list args;
        va_start(args, format);
        int size = vsnprintf(NULL, 0, format, args);
        va_end(args);
        
        if (size > 0) {
            *out_err = xllr_alloc_string(NULL, size + 1);
            if (*out_err) {
                va_start(args, format);
                vsnprintf(*out_err, size + 1, format, args);
                va_end(args);
            }
        }
    }
}

// Helper function to check bounds
static int check_bounds(cdts_serializer_t* ser, metaffi_size index, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size max_index;
    if (ser->array_stack) {
        // In array context
        max_index = ser->array_stack->length;
    } else {
        // In root context
        if (!ser->root_cdts) {
            set_error(out_err, "Root CDTS is NULL");
            return CDTS_SER_ERROR_NULL_POINTER;
        }
        max_index = ser->root_cdts->length;
    }
    
    if (index >= max_index) {
        set_error(out_err, "Index out of bounds: %llu >= %llu", (unsigned long long)index, (unsigned long long)max_index);
        return CDTS_SER_ERROR_BOUNDS;
    }
    
    return CDTS_SER_SUCCESS;
}

// Helper function to get current CDT pointer
static struct cdt* get_current_cdt(cdts_serializer_t* ser) {
    if (ser->array_stack) {
        // In array context
        return &ser->array_stack->array_cdts->arr[ser->array_stack->current_index];
    } else {
        // In root context
        return &ser->root_cdts->arr[ser->current_index];
    }
}

// Helper function to advance index
static void advance_index(cdts_serializer_t* ser) {
    if (ser->array_stack) {
        ser->array_stack->current_index++;
    } else {
        ser->current_index++;
    }
}

// Helper function to validate type
static int validate_type(cdts_serializer_t* ser, metaffi_type expected, char** out_err) {
    struct cdt* cdt = get_current_cdt(ser);
    if (cdt->type != expected) {
        set_error(out_err, "Type mismatch: expected %llu, got %llu", (unsigned long long)expected, (unsigned long long)cdt->type);
        return CDTS_SER_ERROR_TYPE_MISMATCH;
    }
    return CDTS_SER_SUCCESS;
}

// Helper function to get metaffi type from C type (for validation)
static metaffi_type get_metaffi_type_for_int8(void) { return metaffi_int8_type; }
static metaffi_type get_metaffi_type_for_int16(void) { return metaffi_int16_type; }
static metaffi_type get_metaffi_type_for_int32(void) { return metaffi_int32_type; }
static metaffi_type get_metaffi_type_for_int64(void) { return metaffi_int64_type; }
static metaffi_type get_metaffi_type_for_uint8(void) { return metaffi_uint8_type; }
static metaffi_type get_metaffi_type_for_uint16(void) { return metaffi_uint16_type; }
static metaffi_type get_metaffi_type_for_uint32(void) { return metaffi_uint32_type; }
static metaffi_type get_metaffi_type_for_uint64(void) { return metaffi_uint64_type; }
static metaffi_type get_metaffi_type_for_float32(void) { return metaffi_float32_type; }
static metaffi_type get_metaffi_type_for_float64(void) { return metaffi_float64_type; }
static metaffi_type get_metaffi_type_for_bool(void) { return metaffi_bool_type; }

// ===== LIFECYCLE =====

cdts_serializer_t* cdts_ser_create(struct cdts* data, char** out_err) {
    if (!data) {
        set_error(out_err, "CDTS data is NULL");
        return NULL;
    }
    
    cdts_serializer_t* ser = (cdts_serializer_t*)xllr_alloc_memory(sizeof(cdts_serializer_t));
    if (!ser) {
        set_error(out_err, "Failed to allocate serializer");
        return NULL;
    }
    
    ser->root_cdts = data;
    ser->current_index = 0;
    ser->array_stack = NULL;
    
    return ser;
}

void cdts_ser_destroy(cdts_serializer_t* ser) {
    if (!ser) return;
    
    // Free array stack
    while (ser->array_stack) {
        array_context_t* next = ser->array_stack->next;
        xllr_free_memory(ser->array_stack);
        ser->array_stack = next;
    }
    
    xllr_free_memory(ser);
}

int cdts_ser_reset(cdts_serializer_t* ser, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    ser->current_index = 0;
    
    // Clear array stack
    while (ser->array_stack) {
        array_context_t* next = ser->array_stack->next;
        xllr_free_memory(ser->array_stack);
        ser->array_stack = next;
    }
    
    return CDTS_SER_SUCCESS;
}

// ===== SERIALIZATION (ADD FUNCTIONS) =====

static int add_primitive(cdts_serializer_t* ser, metaffi_type type, void* val_ptr, size_t val_size, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = type;
    cdt->free_required = false;
    memcpy(&cdt->cdt_val, val_ptr, val_size);
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_int8(cdts_serializer_t* ser, int8_t val, char** out_err) {
    return add_primitive(ser, metaffi_int8_type, &val, sizeof(int8_t), out_err);
}

int cdts_ser_add_int16(cdts_serializer_t* ser, int16_t val, char** out_err) {
    return add_primitive(ser, metaffi_int16_type, &val, sizeof(int16_t), out_err);
}

int cdts_ser_add_int32(cdts_serializer_t* ser, int32_t val, char** out_err) {
    return add_primitive(ser, metaffi_int32_type, &val, sizeof(int32_t), out_err);
}

int cdts_ser_add_int64(cdts_serializer_t* ser, int64_t val, char** out_err) {
    return add_primitive(ser, metaffi_int64_type, &val, sizeof(int64_t), out_err);
}

int cdts_ser_add_uint8(cdts_serializer_t* ser, uint8_t val, char** out_err) {
    return add_primitive(ser, metaffi_uint8_type, &val, sizeof(uint8_t), out_err);
}

int cdts_ser_add_uint16(cdts_serializer_t* ser, uint16_t val, char** out_err) {
    return add_primitive(ser, metaffi_uint16_type, &val, sizeof(uint16_t), out_err);
}

int cdts_ser_add_uint32(cdts_serializer_t* ser, uint32_t val, char** out_err) {
    return add_primitive(ser, metaffi_uint32_type, &val, sizeof(uint32_t), out_err);
}

int cdts_ser_add_uint64(cdts_serializer_t* ser, uint64_t val, char** out_err) {
    return add_primitive(ser, metaffi_uint64_type, &val, sizeof(uint64_t), out_err);
}

int cdts_ser_add_float32(cdts_serializer_t* ser, float val, char** out_err) {
    return add_primitive(ser, metaffi_float32_type, &val, sizeof(float), out_err);
}

int cdts_ser_add_float64(cdts_serializer_t* ser, double val, char** out_err) {
    return add_primitive(ser, metaffi_float64_type, &val, sizeof(double), out_err);
}

int cdts_ser_add_bool(cdts_serializer_t* ser, bool val, char** out_err) {
    metaffi_bool bval = val ? 1 : 0;
    return add_primitive(ser, metaffi_bool_type, &bval, sizeof(metaffi_bool), out_err);
}

int cdts_ser_add_string8(cdts_serializer_t* ser, const char* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_string8_type;
    
    if (val) {
        size_t len = strlen(val);
        char8_t* copy = xllr_alloc_string8((const char8_t*)val, len);
        if (!copy) {
            set_error(out_err, "Failed to allocate string memory");
            return CDTS_SER_ERROR_MEMORY;
        }
        cdt->cdt_val.string8_val = copy;
        cdt->free_required = true;
    } else {
        cdt->cdt_val.string8_val = NULL;
        cdt->free_required = false;
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_string16(cdts_serializer_t* ser, const char16_t* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_string16_type;
    
    if (val) {
        size_t len = 0;
        while (val[len] != 0) len++;
        char16_t* copy = xllr_alloc_string16(val, len);
        if (!copy) {
            set_error(out_err, "Failed to allocate string memory");
            return CDTS_SER_ERROR_MEMORY;
        }
        cdt->cdt_val.string16_val = copy;
        cdt->free_required = true;
    } else {
        cdt->cdt_val.string16_val = NULL;
        cdt->free_required = false;
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_string32(cdts_serializer_t* ser, const char32_t* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_string32_type;
    
    if (val) {
        size_t len = 0;
        while (val[len] != 0) len++;
        char32_t* copy = xllr_alloc_string32(val, len);
        if (!copy) {
            set_error(out_err, "Failed to allocate string memory");
            return CDTS_SER_ERROR_MEMORY;
        }
        cdt->cdt_val.string32_val = copy;
        cdt->free_required = true;
    } else {
        cdt->cdt_val.string32_val = NULL;
        cdt->free_required = false;
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_char8(cdts_serializer_t* ser, const struct metaffi_char8* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_char8_type;
    cdt->cdt_val.char8_val = *val;
    cdt->free_required = false;
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_char16(cdts_serializer_t* ser, const struct metaffi_char16* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_char16_type;
    cdt->cdt_val.char16_val = *val;
    cdt->free_required = false;
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_char32(cdts_serializer_t* ser, const struct metaffi_char32* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_char32_type;
    cdt->cdt_val.char32_val = *val;
    cdt->free_required = false;
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_handle(cdts_serializer_t* ser, const struct cdt_metaffi_handle* handle, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!handle) {
        set_error(out_err, "Handle is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_handle_type;
    // Allocate handle copy
    struct cdt_metaffi_handle* handle_copy = (struct cdt_metaffi_handle*)xllr_alloc_memory(sizeof(struct cdt_metaffi_handle));
    if (!handle_copy) {
        set_error(out_err, "Failed to allocate handle memory");
        return CDTS_SER_ERROR_MEMORY;
    }
    *handle_copy = *handle;
    cdt->cdt_val.handle_val = handle_copy;
    cdt->free_required = false;  // Handle lifetime managed by user
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_callable(cdts_serializer_t* ser, const struct cdt_metaffi_callable* callable, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!callable) {
        set_error(out_err, "Callable is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_callable_type;
    // Allocate callable copy
    struct cdt_metaffi_callable* callable_copy = (struct cdt_metaffi_callable*)xllr_alloc_memory(sizeof(struct cdt_metaffi_callable));
    if (!callable_copy) {
        set_error(out_err, "Failed to allocate callable memory");
        return CDTS_SER_ERROR_MEMORY;
    }
    callable_copy->val = callable->val;
    callable_copy->params_types_length = callable->params_types_length;
    callable_copy->retval_types_length = callable->retval_types_length;
    
    if (callable->params_types_length > 0) {
        callable_copy->parameters_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * callable->params_types_length);
        if (!callable_copy->parameters_types) {
            xllr_free_memory(callable_copy);
            set_error(out_err, "Failed to allocate callable parameters memory");
            return CDTS_SER_ERROR_MEMORY;
        }
        memcpy(callable_copy->parameters_types, callable->parameters_types, sizeof(metaffi_type) * callable->params_types_length);
    } else {
        callable_copy->parameters_types = NULL;
    }
    
    if (callable->retval_types_length > 0) {
        callable_copy->retval_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * callable->retval_types_length);
        if (!callable_copy->retval_types) {
            xllr_free_memory(callable_copy->parameters_types);
            xllr_free_memory(callable_copy);
            set_error(out_err, "Failed to allocate callable return types memory");
            return CDTS_SER_ERROR_MEMORY;
        }
        memcpy(callable_copy->retval_types, callable->retval_types, sizeof(metaffi_type) * callable->retval_types_length);
    } else {
        callable_copy->retval_types = NULL;
    }
    
    cdt->cdt_val.callable_val = callable_copy;
    cdt->free_required = true;  // CDT owns the callable copy
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_null(cdts_serializer_t* ser, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    cdt->type = metaffi_null_type;
    cdt->free_required = false;
    memset(&cdt->cdt_val, 0, sizeof(cdt->cdt_val));
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

// ===== DESERIALIZATION (GET FUNCTIONS) =====

static int get_primitive(cdts_serializer_t* ser, metaffi_type expected_type, void* val_ptr, size_t val_size, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val_ptr) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, expected_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    memcpy(val_ptr, &cdt->cdt_val, val_size);
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_int8(cdts_serializer_t* ser, int8_t* val, char** out_err) {
    return get_primitive(ser, metaffi_int8_type, val, sizeof(int8_t), out_err);
}

int cdts_ser_get_int16(cdts_serializer_t* ser, int16_t* val, char** out_err) {
    return get_primitive(ser, metaffi_int16_type, val, sizeof(int16_t), out_err);
}

int cdts_ser_get_int32(cdts_serializer_t* ser, int32_t* val, char** out_err) {
    return get_primitive(ser, metaffi_int32_type, val, sizeof(int32_t), out_err);
}

int cdts_ser_get_int64(cdts_serializer_t* ser, int64_t* val, char** out_err) {
    return get_primitive(ser, metaffi_int64_type, val, sizeof(int64_t), out_err);
}

int cdts_ser_get_uint8(cdts_serializer_t* ser, uint8_t* val, char** out_err) {
    return get_primitive(ser, metaffi_uint8_type, val, sizeof(uint8_t), out_err);
}

int cdts_ser_get_uint16(cdts_serializer_t* ser, uint16_t* val, char** out_err) {
    return get_primitive(ser, metaffi_uint16_type, val, sizeof(uint16_t), out_err);
}

int cdts_ser_get_uint32(cdts_serializer_t* ser, uint32_t* val, char** out_err) {
    return get_primitive(ser, metaffi_uint32_type, val, sizeof(uint32_t), out_err);
}

int cdts_ser_get_uint64(cdts_serializer_t* ser, uint64_t* val, char** out_err) {
    return get_primitive(ser, metaffi_uint64_type, val, sizeof(uint64_t), out_err);
}

int cdts_ser_get_float32(cdts_serializer_t* ser, float* val, char** out_err) {
    return get_primitive(ser, metaffi_float32_type, val, sizeof(float), out_err);
}

int cdts_ser_get_float64(cdts_serializer_t* ser, double* val, char** out_err) {
    return get_primitive(ser, metaffi_float64_type, val, sizeof(double), out_err);
}

int cdts_ser_get_bool(cdts_serializer_t* ser, bool* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_bool_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    *val = (cdt->cdt_val.bool_val != 0);
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_string8(cdts_serializer_t* ser, char** val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_string8_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    if (cdt->cdt_val.string8_val) {
        size_t len = strlen((const char*)cdt->cdt_val.string8_val);
        *val = (char*)xllr_alloc_string8(cdt->cdt_val.string8_val, len);
        if (!*val) {
            set_error(out_err, "Failed to allocate string memory");
            return CDTS_SER_ERROR_MEMORY;
        }
    } else {
        *val = NULL;
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_string16(cdts_serializer_t* ser, char16_t** val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_string16_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    if (cdt->cdt_val.string16_val) {
        size_t len = 0;
        while (cdt->cdt_val.string16_val[len] != 0) len++;
        *val = xllr_alloc_string16(cdt->cdt_val.string16_val, len);
        if (!*val) {
            set_error(out_err, "Failed to allocate string memory");
            return CDTS_SER_ERROR_MEMORY;
        }
    } else {
        *val = NULL;
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_string32(cdts_serializer_t* ser, char32_t** val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_string32_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    if (cdt->cdt_val.string32_val) {
        size_t len = 0;
        while (cdt->cdt_val.string32_val[len] != 0) len++;
        *val = xllr_alloc_string32(cdt->cdt_val.string32_val, len);
        if (!*val) {
            set_error(out_err, "Failed to allocate string memory");
            return CDTS_SER_ERROR_MEMORY;
        }
    } else {
        *val = NULL;
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_char8(cdts_serializer_t* ser, struct metaffi_char8* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_char8_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    *val = cdt->cdt_val.char8_val;
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_char16(cdts_serializer_t* ser, struct metaffi_char16* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_char16_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    *val = cdt->cdt_val.char16_val;
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_char32(cdts_serializer_t* ser, struct metaffi_char32* val, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!val) {
        set_error(out_err, "Value pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_char32_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    *val = cdt->cdt_val.char32_val;
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_handle(cdts_serializer_t* ser, struct cdt_metaffi_handle* handle, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!handle) {
        set_error(out_err, "Handle pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_handle_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    if (cdt->cdt_val.handle_val) {
        *handle = *cdt->cdt_val.handle_val;
    } else {
        memset(handle, 0, sizeof(struct cdt_metaffi_handle));
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_callable(cdts_serializer_t* ser, struct cdt_metaffi_callable* callable, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!callable) {
        set_error(out_err, "Callable pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    ret = validate_type(ser, metaffi_callable_type, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    if (cdt->cdt_val.callable_val) {
        callable->val = cdt->cdt_val.callable_val->val;
        callable->params_types_length = cdt->cdt_val.callable_val->params_types_length;
        callable->retval_types_length = cdt->cdt_val.callable_val->retval_types_length;
        
        if (cdt->cdt_val.callable_val->params_types_length > 0) {
            callable->parameters_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * cdt->cdt_val.callable_val->params_types_length);
            if (!callable->parameters_types) {
                set_error(out_err, "Failed to allocate callable parameters memory");
                return CDTS_SER_ERROR_MEMORY;
            }
            memcpy(callable->parameters_types, cdt->cdt_val.callable_val->parameters_types, 
                   sizeof(metaffi_type) * cdt->cdt_val.callable_val->params_types_length);
        } else {
            callable->parameters_types = NULL;
        }
        
        if (cdt->cdt_val.callable_val->retval_types_length > 0) {
            callable->retval_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * cdt->cdt_val.callable_val->retval_types_length);
            if (!callable->retval_types) {
                xllr_free_memory(callable->parameters_types);
                set_error(out_err, "Failed to allocate callable return types memory");
                return CDTS_SER_ERROR_MEMORY;
            }
            memcpy(callable->retval_types, cdt->cdt_val.callable_val->retval_types,
                   sizeof(metaffi_type) * cdt->cdt_val.callable_val->retval_types_length);
        } else {
            callable->retval_types = NULL;
        }
    } else {
        memset(callable, 0, sizeof(struct cdt_metaffi_callable));
    }
    
    advance_index(ser);
    return CDTS_SER_SUCCESS;
}

// ===== ARRAY HANDLING =====

int cdts_ser_add_array_begin(cdts_serializer_t* ser, metaffi_size length, metaffi_type element_type, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    
    // Calculate dimensions: count how deep we are in the array stack
    metaffi_int64 dimensions = 1;
    array_context_t* iter_ctx = ser->array_stack;
    while (iter_ctx) {
        dimensions++;
        iter_ctx = iter_ctx->next;
    }
    
    // Allocate new cdts for the array
    struct cdts* array_cdts = (struct cdts*)xllr_alloc_memory(sizeof(struct cdts));
    if (!array_cdts) {
        set_error(out_err, "Failed to allocate array CDTS");
        return CDTS_SER_ERROR_MEMORY;
    }
    
    array_cdts->length = length;
    array_cdts->fixed_dimensions = dimensions;
    array_cdts->allocated_on_cache = false;
    
    if (length > 0) {
        array_cdts->arr = xllr_alloc_cdt_array(length);
        if (!array_cdts->arr) {
            xllr_free_memory(array_cdts);
            set_error(out_err, "Failed to allocate array elements");
            return CDTS_SER_ERROR_MEMORY;
        }
        // Initialize CDT array to zero
        memset(array_cdts->arr, 0, sizeof(struct cdt) * length);
    } else {
        array_cdts->arr = NULL;
    }
    
    // Set the CDT to point to this array
    cdt->type = metaffi_array_type | element_type;
    cdt->cdt_val.array_val = array_cdts;
    cdt->free_required = true;
    
    // Push onto array stack
    array_context_t* ctx = (array_context_t*)xllr_alloc_memory(sizeof(array_context_t));
    if (!ctx) {
        xllr_free_cdt_array(array_cdts->arr);
        xllr_free_memory(array_cdts);
        set_error(out_err, "Failed to allocate array context");
        return CDTS_SER_ERROR_MEMORY;
    }
    
    ctx->array_cdts = array_cdts;
    ctx->current_index = 0;
    ctx->length = length;
    ctx->element_type = element_type;
    ctx->next = ser->array_stack;
    ser->array_stack = ctx;
    
    return CDTS_SER_SUCCESS;
}

int cdts_ser_add_array_end(cdts_serializer_t* ser, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    if (!ser->array_stack) {
        set_error(out_err, "Not in array context");
        return CDTS_SER_ERROR_INVALID_ARRAY_STATE;
    }
    
    // Verify we've added all elements
    if (ser->array_stack->current_index != ser->array_stack->length) {
        set_error(out_err, "Array not fully populated: %llu/%llu elements", 
                  (unsigned long long)ser->array_stack->current_index, 
                  (unsigned long long)ser->array_stack->length);
        return CDTS_SER_ERROR_INVALID_ARRAY_STATE;
    }
    
    // Pop from array stack
    array_context_t* ctx = ser->array_stack;
    ser->array_stack = ctx->next;
    
    // Advance the parent's index (either root or parent array)
    if (!ser->array_stack) {
        // Back at root level
        ser->current_index++;
    } else {
        // Still in a parent array
        ser->array_stack->current_index++;
    }
    
    xllr_free_memory(ctx);
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_array_begin(cdts_serializer_t* ser, metaffi_size* length, metaffi_type* element_type, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!length || !element_type) {
        set_error(out_err, "Output pointers are NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    struct cdt* cdt = get_current_cdt(ser);
    
    if (!(cdt->type & metaffi_array_type)) {
        set_error(out_err, "Type mismatch: expected array, got %llu", (unsigned long long)cdt->type);
        return CDTS_SER_ERROR_TYPE_MISMATCH;
    }
    
    if (!cdt->cdt_val.array_val) {
        set_error(out_err, "Array value is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    struct cdts* array_cdts = cdt->cdt_val.array_val;
    *length = array_cdts->length;
    *element_type = cdt->type & ~metaffi_array_type;  // Remove array flag to get element type
    
    // Push onto array stack
    array_context_t* ctx = (array_context_t*)xllr_alloc_memory(sizeof(array_context_t));
    if (!ctx) {
        set_error(out_err, "Failed to allocate array context");
        return CDTS_SER_ERROR_MEMORY;
    }
    
    ctx->array_cdts = array_cdts;
    ctx->current_index = 0;
    ctx->length = *length;
    ctx->element_type = *element_type;
    ctx->next = ser->array_stack;
    ser->array_stack = ctx;
    
    return CDTS_SER_SUCCESS;
}

int cdts_ser_get_array_end(cdts_serializer_t* ser, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    if (!ser->array_stack) {
        set_error(out_err, "Not in array context");
        return CDTS_SER_ERROR_INVALID_ARRAY_STATE;
    }
    
    // Verify we've read all elements
    if (ser->array_stack->current_index != ser->array_stack->length) {
        set_error(out_err, "Array not fully read: %llu/%llu elements", 
                  (unsigned long long)ser->array_stack->current_index, 
                  (unsigned long long)ser->array_stack->length);
        return CDTS_SER_ERROR_INVALID_ARRAY_STATE;
    }
    
    // Pop from array stack
    array_context_t* ctx = ser->array_stack;
    ser->array_stack = ctx->next;
    
    // Advance the parent's index (either root or parent array)
    if (!ser->array_stack) {
        // Back at root level
        ser->current_index++;
    } else {
        // Still in a parent array
        ser->array_stack->current_index++;
    }
    
    xllr_free_memory(ctx);
    return CDTS_SER_SUCCESS;
}

// ===== UTILITY FUNCTIONS =====

metaffi_type cdts_ser_peek_type(cdts_serializer_t* ser, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return metaffi_null_type;
    }
    
    metaffi_size index = ser->array_stack ? ser->array_stack->current_index : ser->current_index;
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return metaffi_null_type;
    
    struct cdt* cdt = get_current_cdt(ser);
    return cdt->type;
}

int cdts_ser_is_null(cdts_serializer_t* ser, bool* is_null, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!is_null) {
        set_error(out_err, "Output pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_type type = cdts_ser_peek_type(ser, out_err);
    *is_null = (type == metaffi_null_type);
    return CDTS_SER_SUCCESS;
}

metaffi_size cdts_ser_get_index(cdts_serializer_t* ser, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return 0;
    }
    
    return ser->array_stack ? ser->array_stack->current_index : ser->current_index;
}

int cdts_ser_set_index(cdts_serializer_t* ser, metaffi_size index, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    int ret = check_bounds(ser, index, out_err);
    if (ret != CDTS_SER_SUCCESS) return ret;
    
    if (ser->array_stack) {
        ser->array_stack->current_index = index;
    } else {
        ser->current_index = index;
    }
    
    return CDTS_SER_SUCCESS;
}

metaffi_size cdts_ser_size(cdts_serializer_t* ser, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return 0;
    }
    
    if (ser->array_stack) {
        return ser->array_stack->length;
    } else {
        if (!ser->root_cdts) {
            set_error(out_err, "Root CDTS is NULL");
            return 0;
        }
        return ser->root_cdts->length;
    }
}

int cdts_ser_has_more(cdts_serializer_t* ser, bool* has_more, char** out_err) {
    if (!ser) {
        set_error(out_err, "Serializer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    if (!has_more) {
        set_error(out_err, "Output pointer is NULL");
        return CDTS_SER_ERROR_NULL_POINTER;
    }
    
    metaffi_size current = cdts_ser_get_index(ser, out_err);
    metaffi_size total = cdts_ser_size(ser, out_err);
    *has_more = (current < total);
    return CDTS_SER_SUCCESS;
}
