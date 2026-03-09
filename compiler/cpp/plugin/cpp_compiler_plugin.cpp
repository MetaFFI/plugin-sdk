/**
 * CppCompilerPlugin — implements compiler_plugin_interface for C++.
 *
 * compile_from_host() delegates to CppHostCompiler::compile().
 * compile_to_guest()  is a documented no-op: C++ shared libraries already
 *                     expose all symbols via dlopen/dlsym; no stub is needed.
 */

#include <compiler/compiler_plugin_interface.h>
#include "../host/host_compiler.h"

#include <cstring>
#include <string>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Helper: allocate a C string copy for the error output parameters.
// Caller is responsible for freeing with free().
// ---------------------------------------------------------------------------
static void set_error(const std::string& msg, char** out_err, uint32_t* out_err_len) {
    if (!out_err || !out_err_len) return;

    const size_t len = msg.size();
    *out_err = static_cast<char*>(malloc(len + 1));
    if (*out_err) {
        memcpy(*out_err, msg.c_str(), len + 1);
    }
    *out_err_len = static_cast<uint32_t>(len);
}

// ---------------------------------------------------------------------------
// Plugin implementation
// ---------------------------------------------------------------------------
class CppCompilerPlugin : public compiler_plugin_interface {
public:
    void init() override {
        // No initialisation required
    }

    void compile_from_host(const char* idl_def_json,   uint32_t idl_def_json_length,
                           const char* output_path,    uint32_t output_path_length,
                           const char* host_options,   uint32_t /*host_options_length*/,
                           char** out_err,             uint32_t* out_err_len) override {
        try {
            const std::string json_str(idl_def_json, idl_def_json_length);
            const std::string out_path(output_path,  output_path_length);

            // Derive the stem from host_options if provided, otherwise use "host"
            const std::string stem = (host_options && host_options[0] != '\0')
                                   ? std::string(host_options)
                                   : "host";

            host_compiler_.compile(json_str, out_path, stem);

            // Success — clear error outputs
            if (out_err)     *out_err     = nullptr;
            if (out_err_len) *out_err_len = 0;

        } catch (const std::exception& e) {
            set_error(std::string("CppCompilerPlugin::compile_from_host: ") + e.what(),
                      out_err, out_err_len);
        }
    }

    void compile_to_guest(const char* /*idl_def_json*/, uint32_t /*idl_def_json_length*/,
                          const char* /*output_path*/,  uint32_t /*output_path_length*/,
                          const char* /*guest_options*/,uint32_t /*guest_options_length*/,
                          char** out_err,               uint32_t* out_err_len) override {
        // C++ shared libraries expose all symbols via dlopen/dlsym — no guest stub needed.
        if (out_err)     *out_err     = nullptr;
        if (out_err_len) *out_err_len = 0;
    }

private:
    metaffi::compiler::cpp::CppHostCompiler host_compiler_;
};

// ---------------------------------------------------------------------------
// Factory function for dynamic loading
// ---------------------------------------------------------------------------
extern "C" compiler_plugin_interface* create_compiler_plugin() {
    return new CppCompilerPlugin();
}
