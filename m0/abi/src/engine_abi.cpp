// =============================================================================
// PokoX M0-D — C ABI implementation side.
//
// The C++ implementation lives entirely behind the C header: no std type is
// visible through the boundary, and every entry point is wrapped so a C++
// exception can never unwind into the caller's stack frame (which would be
// undefined behaviour across the ABI, and fatal if the caller is Rust or
// Kotlin/JNI).
// =============================================================================

#include "poko/engine_abi.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <new>

// The engine instance. Private to this translation unit: the public header only
// forward-declares it, so callers cannot depend on its layout.
struct poko_engine {
    std::uint64_t frame_index = 0;
    double total_seconds = 0.0;
    std::uint32_t flags = 0;
};

namespace {

constexpr std::size_t kMinConfigSize =
    sizeof(std::uint32_t) * 4;  // struct_size, abi_version, flags, reserved0

/// Validates the caller's configuration before any field is dereferenced.
int32_t validate_config(const poko_engine_config* config) {
    if (config == nullptr) {
        return POKO_STATUS_INVALID_ARGUMENT;
    }

    // Reject a caller built against an older (smaller) structure: reading fields
    // it did not provide would be an out-of-bounds read.
    if (config->struct_size < kMinConfigSize) {
        return POKO_STATUS_INVALID_CONFIG;
    }
    if (config->struct_size < sizeof(poko_engine_config)) {
        return POKO_STATUS_INVALID_CONFIG;
    }

    if (config->abi_version != POKO_ABI_VERSION) {
        return POKO_STATUS_ABI_MISMATCH;
    }

    if (config->reserved0 != 0) {
        return POKO_STATUS_INVALID_CONFIG;
    }

    // Unknown flag bits mean the caller expects behaviour this binary does not
    // implement; silently ignoring them is how security features get dropped.
    const std::uint32_t known =
        static_cast<std::uint32_t>(POKO_ENGINE_FLAG_HEADLESS);
    if ((config->flags & ~known) != 0) {
        return POKO_STATUS_INVALID_CONFIG;
    }

    return POKO_STATUS_OK;
}

}  // namespace

extern "C" {

uint32_t poko_abi_version(void) {
    return POKO_ABI_VERSION;
}

int32_t poko_engine_create(const poko_engine_config* config, poko_engine** out_engine) {
    // Always clear the out-parameter first: a caller that ignores the status
    // code must still not receive a dangling or uninitialised handle.
    if (out_engine != nullptr) {
        *out_engine = nullptr;
    }

    if (out_engine == nullptr) {
        return POKO_STATUS_INVALID_ARGUMENT;
    }

    const int32_t config_status = validate_config(config);
    if (config_status != POKO_STATUS_OK) {
        return config_status;
    }

    try {
        auto* engine = new (std::nothrow) poko_engine();
        if (engine == nullptr) {
            return POKO_STATUS_OUT_OF_MEMORY;
        }
        engine->flags = config->flags;
        *out_engine = engine;
        return POKO_STATUS_OK;
    } catch (...) {
        return POKO_STATUS_INTERNAL_ERROR;
    }
}

int32_t poko_engine_update(poko_engine* engine, double elapsed_seconds) {
    if (engine == nullptr) {
        return POKO_STATUS_INVALID_ARGUMENT;
    }

    // NaN and infinity propagate into simulation state and are extremely hard
    // to trace back afterwards, so they are rejected at the boundary.
    if (!std::isfinite(elapsed_seconds) || elapsed_seconds < 0.0) {
        return POKO_STATUS_INVALID_ARGUMENT;
    }

    try {
        engine->total_seconds += elapsed_seconds;
        engine->frame_index += 1;
        return POKO_STATUS_OK;
    } catch (...) {
        return POKO_STATUS_INTERNAL_ERROR;
    }
}

int32_t poko_engine_get_frame_index(const poko_engine* engine, uint64_t* out_frame_index) {
    if (out_frame_index != nullptr) {
        *out_frame_index = 0;
    }

    if (engine == nullptr || out_frame_index == nullptr) {
        return POKO_STATUS_INVALID_ARGUMENT;
    }

    try {
        *out_frame_index = engine->frame_index;
        return POKO_STATUS_OK;
    } catch (...) {
        return POKO_STATUS_INTERNAL_ERROR;
    }
}

void poko_engine_destroy(poko_engine* engine) {
    if (engine == nullptr) {
        return;
    }

    try {
        delete engine;
    } catch (...) {
        // A destructor must not throw across the ABI; nothing useful remains
        // to be done besides containing it.
    }
}

const char* poko_status_string(int32_t status) {
    switch (status) {
        case POKO_STATUS_OK: return "ok";
        case POKO_STATUS_END_OF_STREAM: return "end of stream";
        case POKO_STATUS_INVALID_ARGUMENT: return "invalid argument";
        case POKO_STATUS_INVALID_CONFIG: return "invalid config";
        case POKO_STATUS_ABI_MISMATCH: return "abi mismatch";
        case POKO_STATUS_OUT_OF_MEMORY: return "out of memory";
        case POKO_STATUS_NOT_READY: return "not ready";
        case POKO_STATUS_INTERNAL_ERROR: return "internal error";
        default: return "unknown status";
    }
}

}  // extern "C"
