#pragma once

#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for CDTS serializer
 */
typedef struct cdts_serializer cdts_serializer_t;

/**
 * @brief Error codes
 */
#define CDTS_SER_SUCCESS 0
#define CDTS_SER_ERROR_BOUNDS 1
#define CDTS_SER_ERROR_TYPE_MISMATCH 2
#define CDTS_SER_ERROR_NULL_POINTER 3
#define CDTS_SER_ERROR_INVALID_STATE 4
#define CDTS_SER_ERROR_MEMORY 5
#define CDTS_SER_ERROR_INVALID_ARRAY_STATE 6

// ===== LIFECYCLE =====

/**
 * @brief Create a new serializer for the given CDTS
 * @param data Pointer to CDTS structure (serializer does not take ownership)
 * @param out_err Optional output error string (caller must free if set)
 * @return Serializer handle, or NULL on error
 */
cdts_serializer_t* cdts_ser_create(struct cdts* data, char** out_err);

/**
 * @brief Destroy serializer and free resources
 * @param ser Serializer handle (can be NULL)
 */
void cdts_ser_destroy(cdts_serializer_t* ser);

/**
 * @brief Reset serializer index to 0
 * @param ser Serializer handle
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_reset(cdts_serializer_t* ser, char** out_err);

// ===== SERIALIZATION (ADD FUNCTIONS) =====

int cdts_ser_add_int8(cdts_serializer_t* ser, int8_t val, char** out_err);
int cdts_ser_add_int16(cdts_serializer_t* ser, int16_t val, char** out_err);
int cdts_ser_add_int32(cdts_serializer_t* ser, int32_t val, char** out_err);
int cdts_ser_add_int64(cdts_serializer_t* ser, int64_t val, char** out_err);
int cdts_ser_add_uint8(cdts_serializer_t* ser, uint8_t val, char** out_err);
int cdts_ser_add_uint16(cdts_serializer_t* ser, uint16_t val, char** out_err);
int cdts_ser_add_uint32(cdts_serializer_t* ser, uint32_t val, char** out_err);
int cdts_ser_add_uint64(cdts_serializer_t* ser, uint64_t val, char** out_err);
int cdts_ser_add_float32(cdts_serializer_t* ser, float val, char** out_err);
int cdts_ser_add_float64(cdts_serializer_t* ser, double val, char** out_err);
int cdts_ser_add_bool(cdts_serializer_t* ser, bool val, char** out_err);
int cdts_ser_add_string8(cdts_serializer_t* ser, const char* val, char** out_err);
int cdts_ser_add_string16(cdts_serializer_t* ser, const char16_t* val, char** out_err);
int cdts_ser_add_string32(cdts_serializer_t* ser, const char32_t* val, char** out_err);
int cdts_ser_add_char8(cdts_serializer_t* ser, const struct metaffi_char8* val, char** out_err);
int cdts_ser_add_char16(cdts_serializer_t* ser, const struct metaffi_char16* val, char** out_err);
int cdts_ser_add_char32(cdts_serializer_t* ser, const struct metaffi_char32* val, char** out_err);
int cdts_ser_add_handle(cdts_serializer_t* ser, const struct cdt_metaffi_handle* handle, char** out_err);
int cdts_ser_add_callable(cdts_serializer_t* ser, const struct cdt_metaffi_callable* callable, char** out_err);
int cdts_ser_add_null(cdts_serializer_t* ser, char** out_err);

// ===== DESERIALIZATION (GET FUNCTIONS) =====

int cdts_ser_get_int8(cdts_serializer_t* ser, int8_t* val, char** out_err);
int cdts_ser_get_int16(cdts_serializer_t* ser, int16_t* val, char** out_err);
int cdts_ser_get_int32(cdts_serializer_t* ser, int32_t* val, char** out_err);
int cdts_ser_get_int64(cdts_serializer_t* ser, int64_t* val, char** out_err);
int cdts_ser_get_uint8(cdts_serializer_t* ser, uint8_t* val, char** out_err);
int cdts_ser_get_uint16(cdts_serializer_t* ser, uint16_t* val, char** out_err);
int cdts_ser_get_uint32(cdts_serializer_t* ser, uint32_t* val, char** out_err);
int cdts_ser_get_uint64(cdts_serializer_t* ser, uint64_t* val, char** out_err);
int cdts_ser_get_float32(cdts_serializer_t* ser, float* val, char** out_err);
int cdts_ser_get_float64(cdts_serializer_t* ser, double* val, char** out_err);
int cdts_ser_get_bool(cdts_serializer_t* ser, bool* val, char** out_err);
int cdts_ser_get_string8(cdts_serializer_t* ser, char** val, char** out_err);  // Allocates new string, caller must free
int cdts_ser_get_string16(cdts_serializer_t* ser, char16_t** val, char** out_err);
int cdts_ser_get_string32(cdts_serializer_t* ser, char32_t** val, char** out_err);
int cdts_ser_get_char8(cdts_serializer_t* ser, struct metaffi_char8* val, char** out_err);
int cdts_ser_get_char16(cdts_serializer_t* ser, struct metaffi_char16* val, char** out_err);
int cdts_ser_get_char32(cdts_serializer_t* ser, struct metaffi_char32* val, char** out_err);
int cdts_ser_get_handle(cdts_serializer_t* ser, struct cdt_metaffi_handle* handle, char** out_err);
int cdts_ser_get_callable(cdts_serializer_t* ser, struct cdt_metaffi_callable* callable, char** out_err);

// ===== ARRAY HANDLING =====

/**
 * @brief Start serializing an array
 * @param ser Serializer handle
 * @param length Number of elements in the array
 * @param element_type MetaFFI type of array elements
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_add_array_begin(cdts_serializer_t* ser, metaffi_size length, metaffi_type element_type, char** out_err);

/**
 * @brief End serializing an array (pops from array stack)
 * @param ser Serializer handle
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_add_array_end(cdts_serializer_t* ser, char** out_err);

/**
 * @brief Start deserializing an array
 * @param ser Serializer handle
 * @param length Output: number of elements in the array
 * @param element_type Output: MetaFFI type of array elements
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_get_array_begin(cdts_serializer_t* ser, metaffi_size* length, metaffi_type* element_type, char** out_err);

/**
 * @brief End deserializing an array (pops from array stack)
 * @param ser Serializer handle
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_get_array_end(cdts_serializer_t* ser, char** out_err);

// ===== UTILITY FUNCTIONS =====

/**
 * @brief Get the type of the current element without extracting it
 * @param ser Serializer handle
 * @param out_err Optional output error string
 * @return MetaFFI type, or metaffi_null_type on error
 */
metaffi_type cdts_ser_peek_type(cdts_serializer_t* ser, char** out_err);

/**
 * @brief Check if current element is null
 * @param ser Serializer handle
 * @param is_null Output: true if null, false otherwise
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_is_null(cdts_serializer_t* ser, bool* is_null, char** out_err);

/**
 * @brief Get current index
 * @param ser Serializer handle
 * @param out_err Optional output error string
 * @return Current index, or 0 on error
 */
metaffi_size cdts_ser_get_index(cdts_serializer_t* ser, char** out_err);

/**
 * @brief Set current index
 * @param ser Serializer handle
 * @param index New index value
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_set_index(cdts_serializer_t* ser, metaffi_size index, char** out_err);

/**
 * @brief Get total size of CDTS
 * @param ser Serializer handle
 * @param out_err Optional output error string
 * @return Total size, or 0 on error
 */
metaffi_size cdts_ser_size(cdts_serializer_t* ser, char** out_err);

/**
 * @brief Check if more elements are available
 * @param ser Serializer handle
 * @param has_more Output: true if more elements available, false otherwise
 * @param out_err Optional output error string
 * @return CDTS_SER_SUCCESS on success, error code otherwise
 */
int cdts_ser_has_more(cdts_serializer_t* ser, bool* has_more, char** out_err);

#ifdef __cplusplus
}
#endif
