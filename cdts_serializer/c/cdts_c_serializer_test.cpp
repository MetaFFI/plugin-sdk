#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "cdts_c_serializer.h"
#include <runtime/xllr_capi_loader.h>
#include <utils/logger.hpp>
#include <string.h>
#include <math.h>

static auto LOG = metaffi::get_logger("sdk.cdts_serializer.c");

// Initialize xllr before running tests
static void init_xllr() {
    const char* err = load_xllr();
    if (err) {
        METAFFI_CRITICAL(LOG, "FATAL ERROR! Failed to load XLLR C-API. Error: {}", err);
        exit(1);
    }
}

// Use static initialization for C++
struct XllrInitializer {
    XllrInitializer() {
        init_xllr();
    }
};
static XllrInitializer xllr_init;

// Helper function to create a cdts structure in C
static struct cdts* create_cdts(metaffi_size length) {
    struct cdts* cdts = (struct cdts*)xllr_alloc_memory(sizeof(struct cdts));
    if (!cdts) return NULL;
    
    cdts->length = length;
    cdts->fixed_dimensions = 1;
    cdts->allocated_on_cache = false;
    
    if (length > 0) {
        cdts->arr = xllr_alloc_cdt_array(length);
        if (!cdts->arr) {
            xllr_free_memory(cdts);
            return NULL;
        }
        // Initialize to zero
        memset(cdts->arr, 0, sizeof(struct cdt) * length);
    } else {
        cdts->arr = NULL;
    }
    
    return cdts;
}

// Helper function to free a cdts structure
static void free_cdts(struct cdts* cdts) {
    if (!cdts) return;
    
    if (cdts->arr) {
        for (metaffi_size i = 0; i < cdts->length; i++) {
            struct cdt* cdt = &cdts->arr[i];
            if (cdt->free_required) {
                if (cdt->type & metaffi_array_type) {
                    if (cdt->cdt_val.array_val) {
                        // Recursively free nested arrays first
                        free_cdts(cdt->cdt_val.array_val);
                        cdt->cdt_val.array_val = NULL;
                        cdt->free_required = false;
                        cdt->type = metaffi_null_type;
                    }
                } else {
                    switch (cdt->type) {
                    case metaffi_string8_type:
                        if (cdt->cdt_val.string8_val) {
                            // Set flags FIRST to prevent destructor from trying to free
                            cdt->free_required = false;
                            cdt->type = metaffi_null_type;
                            void* str_ptr = cdt->cdt_val.string8_val;
                            cdt->cdt_val.string8_val = NULL;
                            // Now safe to free
                            xllr_free_string((char*)str_ptr);
                        }
                        break;
                    case metaffi_string16_type:
                        if (cdt->cdt_val.string16_val) {
                            cdt->free_required = false;
                            cdt->type = metaffi_null_type;
                            void* str_ptr = cdt->cdt_val.string16_val;
                            cdt->cdt_val.string16_val = NULL;
                            xllr_free_string((char*)str_ptr);
                        }
                        break;
                    case metaffi_string32_type:
                        if (cdt->cdt_val.string32_val) {
                            cdt->free_required = false;
                            cdt->type = metaffi_null_type;
                            void* str_ptr = cdt->cdt_val.string32_val;
                            cdt->cdt_val.string32_val = NULL;
                            xllr_free_string((char*)str_ptr);
                        }
                        break;
                    case metaffi_callable_type:
                        // CDT's free() will handle callable cleanup automatically
                        // No manual cleanup needed here
                        break;
                    }
                }
            }
        }
        
        // Ensure ALL elements are safe before freeing the array
        // This prevents destructors from trying to free xllr-allocated memory
        for (metaffi_size i = 0; i < cdts->length; i++) {
            struct cdt* cdt = &cdts->arr[i];
            // If we missed anything, make sure it's safe
            if (cdt->free_required) {
                cdt->free_required = false;
                cdt->type = metaffi_null_type;
                // Clear pointers to prevent access
                memset(&cdt->cdt_val, 0, sizeof(cdt->cdt_val));
            }
        }
        
        // Now safe to free the array - destructors will see free_required=false
        xllr_free_cdt_array(cdts->arr);
        cdts->arr = NULL;  // Prevent C++ destructor from trying to free
    }
    cdts->length = 0;  // Mark as empty
    cdts->fixed_dimensions = 0;
    cdts->allocated_on_cache = 0;
    xllr_free_memory(cdts);
}

// Helper function to set string in CDT (C version)
static void set_cdt_string8(struct cdt* cdt, const char* str) {
    cdt->type = metaffi_string8_type;
    if (str) {
        size_t len = strlen(str);
        char8_t* copy = xllr_alloc_string8((const char8_t*)str, len);
        cdt->cdt_val.string8_val = copy;
        cdt->free_required = true;
    } else {
        cdt->cdt_val.string8_val = NULL;
        cdt->free_required = false;
    }
}

static void set_cdt_string16(struct cdt* cdt, const char16_t* str) {
    cdt->type = metaffi_string16_type;
    if (str) {
        size_t len = 0;
        while (str[len] != 0) len++;
        char16_t* copy = xllr_alloc_string16(str, len);
        cdt->cdt_val.string16_val = copy;
        cdt->free_required = true;
    } else {
        cdt->cdt_val.string16_val = NULL;
        cdt->free_required = false;
    }
}

static void set_cdt_string32(struct cdt* cdt, const char32_t* str) {
    cdt->type = metaffi_string32_type;
    if (str) {
        size_t len = 0;
        while (str[len] != 0) len++;
        char32_t* copy = xllr_alloc_string32(str, len);
        cdt->cdt_val.string32_val = copy;
        cdt->free_required = true;
    } else {
        cdt->cdt_val.string32_val = NULL;
        cdt->free_required = false;
    }
}

// Helper function to create array in CDT (C version)
static struct cdts* create_array_cdts(metaffi_size length, metaffi_int64 dimensions, metaffi_type element_type) {
    struct cdts* array_cdts = (struct cdts*)xllr_alloc_memory(sizeof(struct cdts));
    if (!array_cdts) return NULL;
    
    array_cdts->length = length;
    array_cdts->fixed_dimensions = dimensions;
    array_cdts->allocated_on_cache = false;
    
    if (length > 0) {
        array_cdts->arr = xllr_alloc_cdt_array(length);
        if (!array_cdts->arr) {
            xllr_free_memory(array_cdts);
            return NULL;
        }
        // Initialize to zero
        memset(array_cdts->arr, 0, sizeof(struct cdt) * length);
    } else {
        array_cdts->arr = NULL;
    }
    
    return array_cdts;
}

static void set_cdt_array(struct cdt* cdt, struct cdts* array_cdts, metaffi_type element_type) {
    cdt->type = metaffi_array_type | element_type;
    cdt->cdt_val.array_val = array_cdts;
    cdt->free_required = true;
}

TEST_SUITE("CDTS C Serializer") {
    TEST_CASE("Serialize and deserialize int8") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        int8_t original = -42;
        CHECK(cdts_ser_add_int8(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        int8_t extracted;
        CHECK(cdts_ser_get_int8(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize int16") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        int16_t original = -12345;
        CHECK(cdts_ser_add_int16(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        int16_t extracted;
        CHECK(cdts_ser_get_int16(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize int32") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        int32_t original = -123456789;
        CHECK(cdts_ser_add_int32(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        int32_t extracted;
        CHECK(cdts_ser_get_int32(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize int64") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        int64_t original = -1234567890123456LL;
        CHECK(cdts_ser_add_int64(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        int64_t extracted;
        CHECK(cdts_ser_get_int64(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize uint8") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        uint8_t original = 255;
        CHECK(cdts_ser_add_uint8(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        uint8_t extracted;
        CHECK(cdts_ser_get_uint8(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize uint16") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        uint16_t original = 65535;
        CHECK(cdts_ser_add_uint16(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        uint16_t extracted;
        CHECK(cdts_ser_get_uint16(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize uint32") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        uint32_t original = 4294967295U;
        CHECK(cdts_ser_add_uint32(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        uint32_t extracted;
        CHECK(cdts_ser_get_uint32(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize uint64") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        uint64_t original = 18446744073709551615ULL;
        CHECK(cdts_ser_add_uint64(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        uint64_t extracted;
        CHECK(cdts_ser_get_uint64(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == original);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize float32") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        float original = 3.14159f;
        CHECK(cdts_ser_add_float32(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        float extracted;
        CHECK(cdts_ser_get_float32(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == doctest::Approx(original));
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize float64") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        double original = 2.71828182845904523536;
        CHECK(cdts_ser_add_float64(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        double extracted;
        CHECK(cdts_ser_get_float64(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted == doctest::Approx(original));
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize bool") {
        struct cdts* data = create_cdts(2);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_bool(ser, true, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_bool(ser, false, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        bool b1, b2;
        CHECK(cdts_ser_get_bool(ser, &b1, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_bool(ser, &b2, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(b1 == true);
        CHECK(b2 == false);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize multiple primitives") {
        struct cdts* data = create_cdts(5);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_int32(ser, 42, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_float32(ser, 3.14f, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_float64(ser, 2.71828, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_bool(ser, true, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_uint64(ser, 999, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        int32_t i;
        float f;
        double d;
        bool b;
        uint64_t u;
        
        CHECK(cdts_ser_get_int32(ser, &i, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_float32(ser, &f, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_float64(ser, &d, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_bool(ser, &b, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_uint64(ser, &u, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(i == 42);
        CHECK(f == doctest::Approx(3.14f));
        CHECK(d == doctest::Approx(2.71828));
        CHECK(b == true);
        CHECK(u == 999);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize string8") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        const char* original = "Hello, MetaFFI!";
        CHECK(cdts_ser_add_string8(ser, original, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        char* extracted;
        CHECK(cdts_ser_get_string8(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(strcmp(extracted, original) == 0);
        
        xllr_free_string(extracted);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize empty string") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);

        const char* original = "";
        CHECK(cdts_ser_add_string8(ser, original, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        char* extracted;
        CHECK(cdts_ser_get_string8(ser, &extracted, NULL) == CDTS_SER_SUCCESS);

        CHECK(strcmp(extracted, original) == 0);

        xllr_free_string(extracted);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }

    TEST_CASE("Serialize and deserialize string16") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);

        const char16_t* original = u"Hello UTF-16";
        CHECK(cdts_ser_add_string16(ser, original, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        char16_t* extracted;
        CHECK(cdts_ser_get_string16(ser, &extracted, NULL) == CDTS_SER_SUCCESS);

        // Compare UTF-16 strings
        size_t i = 0;
        while (original[i] != 0 && extracted[i] != 0) {
            CHECK(extracted[i] == original[i]);
            i++;
        }
        CHECK(original[i] == 0);
        CHECK(extracted[i] == 0);

        xllr_free_string((char*)extracted);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }

    TEST_CASE("Serialize and deserialize string32") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);

        const char32_t* original = U"Hello UTF-32";
        CHECK(cdts_ser_add_string32(ser, original, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        char32_t* extracted;
        CHECK(cdts_ser_get_string32(ser, &extracted, NULL) == CDTS_SER_SUCCESS);

        // Compare UTF-32 strings
        size_t i = 0;
        while (original[i] != 0 && extracted[i] != 0) {
            CHECK(extracted[i] == original[i]);
            i++;
        }
        CHECK(original[i] == 0);
        CHECK(extracted[i] == 0);

        xllr_free_string((char*)extracted);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize 1D array of int32") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        int32_t original[] = {1, 2, 3, 4, 5};
        size_t original_len = 5;
        
        CHECK(cdts_ser_add_array_begin(ser, original_len, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        for (size_t i = 0; i < original_len; i++) {
            CHECK(cdts_ser_add_int32(ser, original[i], NULL) == CDTS_SER_SUCCESS);
        }
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        metaffi_size length;
        metaffi_type element_type;
        CHECK(cdts_ser_get_array_begin(ser, &length, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length == original_len);
        
        int32_t* extracted = (int32_t*)xllr_alloc_memory(length * sizeof(int32_t));
        for (metaffi_size i = 0; i < length; i++) {
            CHECK(cdts_ser_get_int32(ser, &extracted[i], NULL) == CDTS_SER_SUCCESS);
        }
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        for (size_t i = 0; i < original_len; i++) {
            CHECK(extracted[i] == original[i]);
        }
        
        xllr_free_memory(extracted);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize empty array") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_array_begin(ser, 0, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        metaffi_size length;
        metaffi_type element_type;
        CHECK(cdts_ser_get_array_begin(ser, &length, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length == 0);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize null") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_null(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        bool is_null;
        CHECK(cdts_ser_is_null(ser, &is_null, NULL) == CDTS_SER_SUCCESS);
        CHECK(is_null == true);
        CHECK(cdts_ser_peek_type(ser, NULL) == metaffi_null_type);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Type query with peek_type") {
        struct cdts* data = create_cdts(3);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_int32(ser, 42, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_string8(ser, "hello", NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_float64(ser, 3.14, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_peek_type(ser, NULL) == metaffi_int32_type);
        int32_t i;
        CHECK(cdts_ser_get_int32(ser, &i, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_peek_type(ser, NULL) == metaffi_string8_type);
        char* s;
        CHECK(cdts_ser_get_string8(ser, &s, NULL) == CDTS_SER_SUCCESS);
        xllr_free_string(s);
        
        CHECK(cdts_ser_peek_type(ser, NULL) == metaffi_float64_type);
        double d;
        CHECK(cdts_ser_get_float64(ser, &d, NULL) == CDTS_SER_SUCCESS);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Utility methods") {
        struct cdts* data = create_cdts(5);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        metaffi_size index = cdts_ser_get_index(ser, NULL);
        CHECK(index == 0);
        metaffi_size size = cdts_ser_size(ser, NULL);
        CHECK(size == 5);
        bool has_more;
        CHECK(cdts_ser_has_more(ser, &has_more, NULL) == CDTS_SER_SUCCESS);
        CHECK(has_more == true);
        
        CHECK(cdts_ser_add_int32(ser, 1, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 2, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 3, NULL) == CDTS_SER_SUCCESS);
        index = cdts_ser_get_index(ser, NULL);
        CHECK(index == 3);
        CHECK(cdts_ser_has_more(ser, &has_more, NULL) == CDTS_SER_SUCCESS);
        CHECK(has_more == true);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        index = cdts_ser_get_index(ser, NULL);
        CHECK(index == 0);
        
        CHECK(cdts_ser_set_index(ser, 2, NULL) == CDTS_SER_SUCCESS);
        index = cdts_ser_get_index(ser, NULL);
        CHECK(index == 2);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Mixed types serialization/deserialization") {
        struct cdts* data = create_cdts(6);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_int8(ser, 10, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_string8(ser, "hello", NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_float32(ser, 3.14f, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_bool(ser, true, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_uint64(ser, 9999, NULL) == CDTS_SER_SUCCESS);
        // Note: UTF-16 string test omitted for simplicity in C
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        int8_t i8;
        char* s;
        float f32;
        bool b;
        uint64_t u64;
        
        CHECK(cdts_ser_get_int8(ser, &i8, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_string8(ser, &s, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_float32(ser, &f32, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_bool(ser, &b, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_uint64(ser, &u64, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(i8 == 10);
        CHECK(strcmp(s, "hello") == 0);
        CHECK(f32 == doctest::Approx(3.14f));
        CHECK(b == true);
        CHECK(u64 == 9999);
        
        xllr_free_string(s);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    // =============================================================================
    // TYPE PRESERVATION TESTS
    // Verify that CDTS stores the exact type that was serialized
    // =============================================================================

    TEST_CASE("Verify CDTS stores correct integer types after serialization") {
        struct cdts* data = create_cdts(8);
        char* err = NULL;
        cdts_serializer_t* ser = cdts_ser_create(data, &err);

        // Serialize different integer types
        CHECK(cdts_ser_add_int8(ser, -10, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int16(ser, -1000, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, -100000, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int64(ser, -10000000, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_uint8(ser, 10, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_uint16(ser, 1000, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_uint32(ser, 100000, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_uint64(ser, 10000000, &err) == CDTS_SER_SUCCESS);

        // Verify CDTS has correct types stored
        CHECK(data->arr[0].type == metaffi_int8_type);
        CHECK(data->arr[1].type == metaffi_int16_type);
        CHECK(data->arr[2].type == metaffi_int32_type);
        CHECK(data->arr[3].type == metaffi_int64_type);
        CHECK(data->arr[4].type == metaffi_uint8_type);
        CHECK(data->arr[5].type == metaffi_uint16_type);
        CHECK(data->arr[6].type == metaffi_uint32_type);
        CHECK(data->arr[7].type == metaffi_uint64_type);

        cdts_ser_destroy(ser);
        free_cdts(data);
    }

    TEST_CASE("Verify CDTS stores correct float types after serialization") {
        struct cdts* data = create_cdts(2);
        char* err = NULL;
        cdts_serializer_t* ser = cdts_ser_create(data, &err);

        // Serialize different float types
        CHECK(cdts_ser_add_float32(ser, 3.14f, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_float64(ser, 2.71828, &err) == CDTS_SER_SUCCESS);

        // Verify CDTS has correct types stored
        CHECK(data->arr[0].type == metaffi_float32_type);
        CHECK(data->arr[1].type == metaffi_float64_type);

        cdts_ser_destroy(ser);
        free_cdts(data);
    }

    TEST_CASE("Verify CDTS stores correct types for mixed primitives") {
        struct cdts* data = create_cdts(5);
        char* err = NULL;
        cdts_serializer_t* ser = cdts_ser_create(data, &err);

        // Serialize mixed types
        CHECK(cdts_ser_add_int32(ser, 42, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_float32(ser, 3.14f, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_float64(ser, 2.71828, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_bool(ser, true, &err) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int16(ser, 1000, &err) == CDTS_SER_SUCCESS);

        // Verify types
        CHECK(data->arr[0].type == metaffi_int32_type);
        CHECK(data->arr[1].type == metaffi_float32_type);
        CHECK(data->arr[2].type == metaffi_float64_type);
        CHECK(data->arr[3].type == metaffi_bool_type);
        CHECK(data->arr[4].type == metaffi_int16_type);

        cdts_ser_destroy(ser);
        free_cdts(data);
    }

    // =============================================================================
    // DESERIALIZATION-ONLY TESTS
    // These simulate receiving pre-filled CDTS from MetaFFI cross-language calls
    // =============================================================================
    
    TEST_CASE("Deserialize pre-filled CDTS with all primitive types") {
        struct cdts* data = create_cdts(11);
        data->arr[0].type = metaffi_int8_type;
        data->arr[0].cdt_val.int8_val = -10;
        data->arr[0].free_required = false;
        
        data->arr[1].type = metaffi_int16_type;
        data->arr[1].cdt_val.int16_val = -1000;
        data->arr[1].free_required = false;
        
        data->arr[2].type = metaffi_int32_type;
        data->arr[2].cdt_val.int32_val = -100000;
        data->arr[2].free_required = false;
        
        data->arr[3].type = metaffi_int64_type;
        data->arr[3].cdt_val.int64_val = -10000000;
        data->arr[3].free_required = false;
        
        data->arr[4].type = metaffi_uint8_type;
        data->arr[4].cdt_val.uint8_val = 10;
        data->arr[4].free_required = false;
        
        data->arr[5].type = metaffi_uint16_type;
        data->arr[5].cdt_val.uint16_val = 1000;
        data->arr[5].free_required = false;
        
        data->arr[6].type = metaffi_uint32_type;
        data->arr[6].cdt_val.uint32_val = 100000;
        data->arr[6].free_required = false;
        
        data->arr[7].type = metaffi_uint64_type;
        data->arr[7].cdt_val.uint64_val = 10000000;
        data->arr[7].free_required = false;
        
        data->arr[8].type = metaffi_float32_type;
        data->arr[8].cdt_val.float32_val = 3.14f;
        data->arr[8].free_required = false;
        
        data->arr[9].type = metaffi_float64_type;
        data->arr[9].cdt_val.float64_val = 2.71828;
        data->arr[9].free_required = false;
        
        data->arr[10].type = metaffi_bool_type;
        data->arr[10].cdt_val.bool_val = 1;
        data->arr[10].free_required = false;
        
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        int8_t i8; int16_t i16; int32_t i32; int64_t i64;
        uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
        float f; double d; bool b;
        
        CHECK(cdts_ser_get_int8(deser, &i8, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_int16(deser, &i16, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_int32(deser, &i32, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_int64(deser, &i64, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_uint8(deser, &u8, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_uint16(deser, &u16, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_uint32(deser, &u32, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_uint64(deser, &u64, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_float32(deser, &f, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_float64(deser, &d, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_bool(deser, &b, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(i8 == -10);
        CHECK(i16 == -1000);
        CHECK(i32 == -100000);
        CHECK(i64 == -10000000);
        CHECK(u8 == 10);
        CHECK(u16 == 1000);
        CHECK(u32 == 100000);
        CHECK(u64 == 10000000);
        CHECK(f == doctest::Approx(3.14f));
        CHECK(d == doctest::Approx(2.71828));
        CHECK(b == true);
        
        cdts_ser_destroy(deser);
        free_cdts(data);
    }
    
    TEST_CASE("Deserialize pre-filled CDTS with strings") {
        struct cdts* data = create_cdts(3);
        set_cdt_string8(&data->arr[0], "Hello UTF-8");
        set_cdt_string16(&data->arr[1], u"Hello UTF-16");
        set_cdt_string32(&data->arr[2], U"Hello UTF-32");
        
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        char* s8;
        char16_t* s16;
        char32_t* s32;
        
        CHECK(cdts_ser_get_string8(deser, &s8, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_string16(deser, &s16, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_string32(deser, &s32, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(strcmp(s8, "Hello UTF-8") == 0);
        // Note: UTF-16/32 comparison simplified
        xllr_free_string(s8);
        xllr_free_string((char*)s16);
        xllr_free_string((char*)s32);
        
        cdts_ser_destroy(deser);
        free_cdts(data);
    }
    
    TEST_CASE("Deserialize pre-filled CDTS with 1D array") {
        struct cdts* data = create_cdts(1);
        struct cdts* array_cdts = create_array_cdts(5, 1, metaffi_int32_type);
        array_cdts->arr[0].type = metaffi_int32_type;
        array_cdts->arr[0].cdt_val.int32_val = 10;
        array_cdts->arr[1].type = metaffi_int32_type;
        array_cdts->arr[1].cdt_val.int32_val = 20;
        array_cdts->arr[2].type = metaffi_int32_type;
        array_cdts->arr[2].cdt_val.int32_val = 30;
        array_cdts->arr[3].type = metaffi_int32_type;
        array_cdts->arr[3].cdt_val.int32_val = 40;
        array_cdts->arr[4].type = metaffi_int32_type;
        array_cdts->arr[4].cdt_val.int32_val = 50;
        set_cdt_array(&data->arr[0], array_cdts, metaffi_int32_type);

        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        metaffi_size length;
        metaffi_type element_type;
        CHECK(cdts_ser_get_array_begin(deser, &length, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length == 5);

        int32_t result[5];
        for (metaffi_size i = 0; i < length; i++) {
            CHECK(cdts_ser_get_int32(deser, &result[i], NULL) == CDTS_SER_SUCCESS);
        }
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        CHECK(result[0] == 10);
        CHECK(result[1] == 20);
        CHECK(result[2] == 30);
        CHECK(result[3] == 40);
        CHECK(result[4] == 50);

        cdts_ser_destroy(deser);
        free_cdts(data);
    }

    TEST_CASE("Deserialize pre-filled CDTS with 2D array") {
        // Manually create 2D array: [[1, 2, 3], [4, 5, 6]]
        struct cdts* data = create_cdts(1);
        struct cdts* arr2d = create_array_cdts(2, 2, metaffi_int32_type);

        // First row
        struct cdts* row1 = create_array_cdts(3, 1, metaffi_int32_type);
        row1->arr[0].type = metaffi_int32_type;
        row1->arr[0].cdt_val.int32_val = 1;
        row1->arr[1].type = metaffi_int32_type;
        row1->arr[1].cdt_val.int32_val = 2;
        row1->arr[2].type = metaffi_int32_type;
        row1->arr[2].cdt_val.int32_val = 3;
        set_cdt_array(&arr2d->arr[0], row1, metaffi_int32_type);

        // Second row
        struct cdts* row2 = create_array_cdts(3, 1, metaffi_int32_type);
        row2->arr[0].type = metaffi_int32_type;
        row2->arr[0].cdt_val.int32_val = 4;
        row2->arr[1].type = metaffi_int32_type;
        row2->arr[1].cdt_val.int32_val = 5;
        row2->arr[2].type = metaffi_int32_type;
        row2->arr[2].cdt_val.int32_val = 6;
        set_cdt_array(&arr2d->arr[1], row2, metaffi_int32_type);

        set_cdt_array(&data->arr[0], arr2d, metaffi_int32_type);

        // Deserialize
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        metaffi_size length1, length2;
        metaffi_type element_type;

        CHECK(cdts_ser_get_array_begin(deser, &length1, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length1 == 2);

        // First row
        CHECK(cdts_ser_get_array_begin(deser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 3);
        int32_t val;
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 1);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 2);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 3);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        // Second row
        CHECK(cdts_ser_get_array_begin(deser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 3);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 4);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 5);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 6);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        cdts_ser_destroy(deser);
        free_cdts(data);
    }

    TEST_CASE("Deserialize pre-filled CDTS with 3D array") {
        // Manually create 3D array: [[[1, 2]], [[3, 4]]]
        struct cdts* data = create_cdts(1);
        struct cdts* arr3d = create_array_cdts(2, 3, metaffi_int32_type);

        // First 2D element
        struct cdts* arr2d_0 = create_array_cdts(1, 2, metaffi_int32_type);
        struct cdts* row_0_0 = create_array_cdts(2, 1, metaffi_int32_type);
        row_0_0->arr[0].type = metaffi_int32_type;
        row_0_0->arr[0].cdt_val.int32_val = 1;
        row_0_0->arr[1].type = metaffi_int32_type;
        row_0_0->arr[1].cdt_val.int32_val = 2;
        set_cdt_array(&arr2d_0->arr[0], row_0_0, metaffi_int32_type);
        set_cdt_array(&arr3d->arr[0], arr2d_0, metaffi_int32_type);

        // Second 2D element
        struct cdts* arr2d_1 = create_array_cdts(1, 2, metaffi_int32_type);
        struct cdts* row_1_0 = create_array_cdts(2, 1, metaffi_int32_type);
        row_1_0->arr[0].type = metaffi_int32_type;
        row_1_0->arr[0].cdt_val.int32_val = 3;
        row_1_0->arr[1].type = metaffi_int32_type;
        row_1_0->arr[1].cdt_val.int32_val = 4;
        set_cdt_array(&arr2d_1->arr[0], row_1_0, metaffi_int32_type);
        set_cdt_array(&arr3d->arr[1], arr2d_1, metaffi_int32_type);

        set_cdt_array(&data->arr[0], arr3d, metaffi_int32_type);

        // Deserialize
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        metaffi_size length1, length2, length3;
        metaffi_type element_type;

        CHECK(cdts_ser_get_array_begin(deser, &length1, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length1 == 2);

        // First 2D element
        CHECK(cdts_ser_get_array_begin(deser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 1);
        CHECK(cdts_ser_get_array_begin(deser, &length3, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length3 == 2);
        int32_t val;
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 1);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 2);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        // Second 2D element
        CHECK(cdts_ser_get_array_begin(deser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 1);
        CHECK(cdts_ser_get_array_begin(deser, &length3, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length3 == 2);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 3);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 4);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        cdts_ser_destroy(deser);
        free_cdts(data);
    }

    TEST_CASE("Deserialize pre-filled CDTS with ragged array") {
        // Manually create ragged array: [[1], [2, 3], [4, 5, 6]]
        struct cdts* data = create_cdts(1);
        struct cdts* arr = create_array_cdts(3, 2, metaffi_int32_type);

        // First row (1 element)
        struct cdts* row1 = create_array_cdts(1, 1, metaffi_int32_type);
        row1->arr[0].type = metaffi_int32_type;
        row1->arr[0].cdt_val.int32_val = 1;
        set_cdt_array(&arr->arr[0], row1, metaffi_int32_type);

        // Second row (2 elements)
        struct cdts* row2 = create_array_cdts(2, 1, metaffi_int32_type);
        row2->arr[0].type = metaffi_int32_type;
        row2->arr[0].cdt_val.int32_val = 2;
        row2->arr[1].type = metaffi_int32_type;
        row2->arr[1].cdt_val.int32_val = 3;
        set_cdt_array(&arr->arr[1], row2, metaffi_int32_type);

        // Third row (3 elements)
        struct cdts* row3 = create_array_cdts(3, 1, metaffi_int32_type);
        row3->arr[0].type = metaffi_int32_type;
        row3->arr[0].cdt_val.int32_val = 4;
        row3->arr[1].type = metaffi_int32_type;
        row3->arr[1].cdt_val.int32_val = 5;
        row3->arr[2].type = metaffi_int32_type;
        row3->arr[2].cdt_val.int32_val = 6;
        set_cdt_array(&arr->arr[2], row3, metaffi_int32_type);

        set_cdt_array(&data->arr[0], arr, metaffi_int32_type);

        // Deserialize
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        metaffi_size length1, length2;
        metaffi_type element_type;

        CHECK(cdts_ser_get_array_begin(deser, &length1, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length1 == 3);

        // First row (1 element)
        CHECK(cdts_ser_get_array_begin(deser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 1);
        int32_t val;
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 1);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        // Second row (2 elements)
        CHECK(cdts_ser_get_array_begin(deser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 2);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 2);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 3);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        // Third row (3 elements)
        CHECK(cdts_ser_get_array_begin(deser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 3);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 4);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 5);
        CHECK(cdts_ser_get_int32(deser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 6);
        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_get_array_end(deser, NULL) == CDTS_SER_SUCCESS);

        cdts_ser_destroy(deser);
        free_cdts(data);
    }
    
    TEST_CASE("Deserialize pre-filled CDTS with NULL value") {
        struct cdts* data = create_cdts(1);
        data->arr[0].type = metaffi_null_type;
        data->arr[0].free_required = false;
        
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        bool is_null;
        CHECK(cdts_ser_is_null(deser, &is_null, NULL) == CDTS_SER_SUCCESS);
        CHECK(is_null == true);
        CHECK(cdts_ser_peek_type(deser, NULL) == metaffi_null_type);
        
        cdts_ser_destroy(deser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize 2D array") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        // Create 2D array: [[1, 2, 3], [4, 5, 6]]
        CHECK(cdts_ser_add_array_begin(ser, 2, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        
        // First row
        CHECK(cdts_ser_add_array_begin(ser, 3, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 1, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 2, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 3, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        // Second row
        CHECK(cdts_ser_add_array_begin(ser, 3, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 4, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 5, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 6, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        
        metaffi_size length1, length2;
        metaffi_type element_type;
        CHECK(cdts_ser_get_array_begin(ser, &length1, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length1 == 2);
        
        // First row
        CHECK(cdts_ser_get_array_begin(ser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 3);
        int32_t val;
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 1);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 2);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 3);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        // Second row
        CHECK(cdts_ser_get_array_begin(ser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 3);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 4);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 5);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 6);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize 1D array of double") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);

        double original[] = {1.1, 2.2, 3.3};
        size_t original_len = 3;

        CHECK(cdts_ser_add_array_begin(ser, original_len, metaffi_float64_type, NULL) == CDTS_SER_SUCCESS);
        for (size_t i = 0; i < original_len; i++) {
            CHECK(cdts_ser_add_float64(ser, original[i], NULL) == CDTS_SER_SUCCESS);
        }
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        metaffi_size length;
        metaffi_type element_type;
        CHECK(cdts_ser_get_array_begin(ser, &length, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length == original_len);

        double* extracted = (double*)xllr_alloc_memory(length * sizeof(double));
        for (metaffi_size i = 0; i < length; i++) {
            CHECK(cdts_ser_get_float64(ser, &extracted[i], NULL) == CDTS_SER_SUCCESS);
        }
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        for (size_t i = 0; i < original_len; i++) {
            CHECK(extracted[i] == doctest::Approx(original[i]));
        }

        xllr_free_memory(extracted);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }

    TEST_CASE("Serialize and deserialize 3D array") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);

        // Create 3D array: [[[1, 2]], [[3, 4]]]
        CHECK(cdts_ser_add_array_begin(ser, 2, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);

        // First 2D element
        CHECK(cdts_ser_add_array_begin(ser, 1, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_begin(ser, 2, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 1, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 2, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        // Second 2D element
        CHECK(cdts_ser_add_array_begin(ser, 1, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_begin(ser, 2, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 3, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 4, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);

        metaffi_size length1, length2, length3;
        metaffi_type element_type;

        // Deserialize 3D array
        CHECK(cdts_ser_get_array_begin(ser, &length1, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length1 == 2);

        // First 2D element
        CHECK(cdts_ser_get_array_begin(ser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 1);
        CHECK(cdts_ser_get_array_begin(ser, &length3, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length3 == 2);
        int32_t val;
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 1);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 2);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        // Second 2D element
        CHECK(cdts_ser_get_array_begin(ser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 1);
        CHECK(cdts_ser_get_array_begin(ser, &length3, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length3 == 2);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 3);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 4);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);

        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Error: Type mismatch on deserialization") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_int32(ser, 42, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        
        char* s;
        int ret = cdts_ser_get_string8(ser, &s, NULL);
        CHECK(ret == CDTS_SER_ERROR_TYPE_MISMATCH);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Error: Bounds violation on serialization") {
        struct cdts* data = create_cdts(2);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_int32(ser, 1, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 2, NULL) == CDTS_SER_SUCCESS);
        
        int ret = cdts_ser_add_int32(ser, 3, NULL);
        CHECK(ret == CDTS_SER_ERROR_BOUNDS);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Error: Bounds violation on deserialization") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);

        CHECK(cdts_ser_add_int32(ser, 42, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);

        int32_t i1, i2;
        CHECK(cdts_ser_get_int32(ser, &i1, NULL) == CDTS_SER_SUCCESS);

        int ret = cdts_ser_get_int32(ser, &i2, NULL);
        CHECK(ret == CDTS_SER_ERROR_BOUNDS);

        cdts_ser_destroy(ser);
        free_cdts(data);
    }

    TEST_CASE("Error: Peek type beyond bounds") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);

        CHECK(cdts_ser_add_int32(ser, 42, NULL) == CDTS_SER_SUCCESS);

        // Index is now at 1, which is out of bounds
        // peek_type should return error or throw
        char* err = NULL;
        metaffi_type type = cdts_ser_peek_type(ser, &err);

        // Should either return error type or set error string
        CHECK((type == metaffi_null_type || err != NULL));

        if (err) {
            xllr_free_string(err);
        }

        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Deserialization from existing CDTS") {
        struct cdts* data = create_cdts(3);
        data->arr[0].type = metaffi_int32_type;
        data->arr[0].cdt_val.int32_val = 123;
        data->arr[0].free_required = false;
        set_cdt_string8(&data->arr[1], "test");
        data->arr[2].type = metaffi_float64_type;
        data->arr[2].cdt_val.float64_val = 9.99;
        data->arr[2].free_required = false;
        
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        int32_t i;
        char* s;
        double d;
        
        CHECK(cdts_ser_get_int32(ser, &i, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_string8(ser, &s, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_float64(ser, &d, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(i == 123);
        CHECK(strcmp(s, "test") == 0);
        CHECK(d == doctest::Approx(9.99));
        
        xllr_free_string(s);
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Chaining operations") {
        struct cdts* data = create_cdts(3);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_add_int32(ser, 1, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 2, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 3, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        int32_t a, b, c;
        CHECK(cdts_ser_get_int32(ser, &a, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_int32(ser, &b, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_get_int32(ser, &c, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(a == 1);
        CHECK(b == 2);
        CHECK(c == 3);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Handle serialization") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        int some_value = 42;
        struct cdt_metaffi_handle handle;
        handle.handle = &some_value;
        handle.runtime_id = 100;
        handle.release = NULL;
        
        CHECK(cdts_ser_add_handle(ser, &handle, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        struct cdt_metaffi_handle extracted;
        CHECK(cdts_ser_get_handle(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted.handle == handle.handle);
        CHECK(extracted.runtime_id == handle.runtime_id);
        CHECK(*((int*)extracted.handle) == 42);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Type query on pre-filled CDTS before deserialization") {
        struct cdts* data = create_cdts(4);
        data->arr[0].type = metaffi_int32_type;
        data->arr[0].cdt_val.int32_val = 42;
        data->arr[0].free_required = false;
        set_cdt_string8(&data->arr[1], "test");
        data->arr[2].type = metaffi_float64_type;
        data->arr[2].cdt_val.float64_val = 3.14;
        data->arr[2].free_required = false;
        data->arr[3].type = metaffi_null_type;
        data->arr[3].free_required = false;
        
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        
        CHECK(cdts_ser_peek_type(deser, NULL) == metaffi_int32_type);
        int32_t i;
        CHECK(cdts_ser_get_int32(deser, &i, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_peek_type(deser, NULL) == metaffi_string8_type);
        char* s;
        CHECK(cdts_ser_get_string8(deser, &s, NULL) == CDTS_SER_SUCCESS);
        xllr_free_string(s);
        
        CHECK(cdts_ser_peek_type(deser, NULL) == metaffi_float64_type);
        double d;
        CHECK(cdts_ser_get_float64(deser, &d, NULL) == CDTS_SER_SUCCESS);
        
        bool is_null;
        CHECK(cdts_ser_is_null(deser, &is_null, NULL) == CDTS_SER_SUCCESS);
        CHECK(is_null == true);
        CHECK(cdts_ser_peek_type(deser, NULL) == metaffi_null_type);
        
        CHECK(i == 42);
        CHECK(d == doctest::Approx(3.14));
        
        cdts_ser_destroy(deser);
        free_cdts(data);
    }
    
    TEST_CASE("Callable serialization") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        metaffi_type param_types[] = {metaffi_int32_type, metaffi_string8_type};
        metaffi_type retval_types[] = {metaffi_float64_type};
        struct cdt_metaffi_callable callable;
        callable.val = (void*)0x1234ABCD;  // Use non-null stubbed value
        callable.parameters_types = param_types;
        callable.params_types_length = 2;
        callable.retval_types = retval_types;
        callable.retval_types_length = 1;
        
        CHECK(cdts_ser_add_callable(ser, &callable, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        struct cdt_metaffi_callable extracted;
        CHECK(cdts_ser_get_callable(ser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted.val == callable.val);
        CHECK(extracted.params_types_length == 2);
        CHECK(extracted.retval_types_length == 1);
        CHECK(extracted.parameters_types[0] == metaffi_int32_type);
        CHECK(extracted.parameters_types[1] == metaffi_string8_type);
        CHECK(extracted.retval_types[0] == metaffi_float64_type);
        
        // No need to manually free - CDT owns the memory and will clean it up
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Serialize and deserialize ragged 2D array") {
        struct cdts* data = create_cdts(1);
        cdts_serializer_t* ser = cdts_ser_create(data, NULL);
        
        // Create ragged array: [[1], [2, 3], [4, 5, 6]]
        CHECK(cdts_ser_add_array_begin(ser, 3, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        
        // First row (1 element)
        CHECK(cdts_ser_add_array_begin(ser, 1, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 1, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        // Second row (2 elements)
        CHECK(cdts_ser_add_array_begin(ser, 2, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 2, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 3, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        // Third row (3 elements)
        CHECK(cdts_ser_add_array_begin(ser, 3, metaffi_int32_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 4, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 5, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_int32(ser, 6, NULL) == CDTS_SER_SUCCESS);
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_add_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_reset(ser, NULL) == CDTS_SER_SUCCESS);
        
        metaffi_size length1, length2;
        metaffi_type element_type;
        CHECK(cdts_ser_get_array_begin(ser, &length1, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length1 == 3);
        
        // First row
        CHECK(cdts_ser_get_array_begin(ser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 1);
        int32_t val;
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 1);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        // Second row
        CHECK(cdts_ser_get_array_begin(ser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 2);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 2);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 3);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        // Third row
        CHECK(cdts_ser_get_array_begin(ser, &length2, &element_type, NULL) == CDTS_SER_SUCCESS);
        CHECK(length2 == 3);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 4);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 5);
        CHECK(cdts_ser_get_int32(ser, &val, NULL) == CDTS_SER_SUCCESS);
        CHECK(val == 6);
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(cdts_ser_get_array_end(ser, NULL) == CDTS_SER_SUCCESS);
        
        cdts_ser_destroy(ser);
        free_cdts(data);
    }
    
    TEST_CASE("Deserialize pre-filled CDTS with handle") {
        struct cdts* data = create_cdts(1);
        int some_value = 99;
        struct cdt_metaffi_handle orig_handle;
        orig_handle.handle = &some_value;
        orig_handle.runtime_id = 123;
        orig_handle.release = NULL;
        
        data->arr[0].type = metaffi_handle_type;
        data->arr[0].cdt_val.handle_val = (struct cdt_metaffi_handle*)xllr_alloc_memory(sizeof(struct cdt_metaffi_handle));
        *data->arr[0].cdt_val.handle_val = orig_handle;
        data->arr[0].free_required = false;
        
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        struct cdt_metaffi_handle extracted;
        CHECK(cdts_ser_get_handle(deser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted.handle == orig_handle.handle);
        CHECK(extracted.runtime_id == 123);
        CHECK(*((int*)extracted.handle) == 99);
        
        xllr_free_memory(data->arr[0].cdt_val.handle_val);
        cdts_ser_destroy(deser);
        free_cdts(data);
    }
    
    TEST_CASE("Deserialize pre-filled CDTS with callable") {
        struct cdts* data = create_cdts(1);
        metaffi_type param_types[] = {metaffi_int32_type, metaffi_bool_type};
        metaffi_type retval_types[] = {metaffi_string8_type};
        
        struct cdt_metaffi_callable* orig_callable = (struct cdt_metaffi_callable*)xllr_alloc_memory(sizeof(struct cdt_metaffi_callable));
        orig_callable->val = (void*)0x5678EFAB;  // Use non-null stubbed value
        orig_callable->params_types_length = 2;
        orig_callable->retval_types_length = 1;
        orig_callable->parameters_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 2);
        orig_callable->parameters_types[0] = metaffi_int32_type;
        orig_callable->parameters_types[1] = metaffi_bool_type;
        orig_callable->retval_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 1);
        orig_callable->retval_types[0] = metaffi_string8_type;
        
        data->arr[0].type = metaffi_callable_type;
        data->arr[0].cdt_val.callable_val = orig_callable;
        data->arr[0].free_required = true;
        
        cdts_serializer_t* deser = cdts_ser_create(data, NULL);
        struct cdt_metaffi_callable extracted;
        CHECK(cdts_ser_get_callable(deser, &extracted, NULL) == CDTS_SER_SUCCESS);
        
        CHECK(extracted.val == orig_callable->val);
        CHECK(extracted.params_types_length == 2);
        CHECK(extracted.retval_types_length == 1);
        CHECK(extracted.parameters_types[0] == metaffi_int32_type);
        CHECK(extracted.parameters_types[1] == metaffi_bool_type);
        CHECK(extracted.retval_types[0] == metaffi_string8_type);
        
        // No need to manually free - CDT owns the memory and will clean it up
        
        cdts_ser_destroy(deser);
        free_cdts(data);
    }
}
