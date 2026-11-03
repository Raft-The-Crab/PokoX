/**
 * @file poko/engine_abi.h
 * @brief PokoX engine C ABI — the only boundary Rust (Windows) and Kotlin/JNI
 *        (Android) are allowed to depend on.
 *
 * Rules enforced by this header (docs/03_ENGINE_ARCHITECTURE.md §5):
 *  - no C++ standard-library type, class, reference or exception crosses the
 *    boundary;
 *  - every entry point is `extern "C"` and returns an integer status;
 *  - opaque handles are incomplete types, so a caller cannot reach into engine
 *    state or depend on its layout;
 *  - structures are versioned by `struct_size` plus `abi_version`, so a mismatch
 *    is rejected instead of being reinterpreted as garbage;
 *  - no function allocates memory the caller must free, except handles returned
 *    through explicit out-parameters that have a matching destroy function.
 *
 * This header is C11 and C++20 compatible and is the *feasibility* surface for
 * M0. M1 replaces it with the full engine ABI; the versioning strategy proven
 * here is what M1 will reuse.
 */
#ifndef POKO_ENGINE_ABI_H
#define POKO_ENGINE_ABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ABI revision implemented by this header. Bumped on any incompatible change. */
#define POKO_ABI_VERSION 1u

/**
 * @brief Status codes returned by every ABI function.
 *
 * Zero means success. Negative values are errors; positive values are reserved
 * for non-fatal "work completed with a caveat" results.
 */
typedef enum poko_status {
    POKO_STATUS_OK = 0,
    POKO_STATUS_END_OF_STREAM = 1,
    POKO_STATUS_INVALID_ARGUMENT = -1,
    POKO_STATUS_INVALID_CONFIG = -2,
    POKO_STATUS_ABI_MISMATCH = -3,
    POKO_STATUS_OUT_OF_MEMORY = -4,
    POKO_STATUS_NOT_READY = -5,
    POKO_STATUS_INTERNAL_ERROR = -100
} poko_status;

/** @brief Opaque engine instance handle. Never dereferenced by the caller. */
typedef struct poko_engine poko_engine;

/**
 * @brief Engine creation parameters.
 *
 * `struct_size` must be `sizeof(poko_engine_config)` as compiled by the caller.
 * Larger values are accepted (the extra bytes belong to a newer caller and are
 * ignored); smaller values are rejected, because the library would otherwise
 * read past the caller's structure.
 */
typedef struct poko_engine_config {
    uint32_t struct_size;  /**< sizeof(poko_engine_config) at the call site. */
    uint32_t abi_version;  /**< Must equal POKO_ABI_VERSION. */
    uint32_t flags;        /**< Reserved bit field; unknown bits are rejected. */
    uint32_t reserved0;    /**< Must be zero. */
} poko_engine_config;

/** @brief Flags accepted in poko_engine_config::flags. */
typedef enum poko_engine_flags {
    POKO_ENGINE_FLAG_NONE = 0u,
    POKO_ENGINE_FLAG_HEADLESS = 1u << 0  /**< Create no window or GPU device. */
} poko_engine_flags;

/**
 * @brief Returns the ABI revision the loaded library implements.
 * @return POKO_ABI_VERSION of the binary, which may differ from the header's
 *         value if caller and library were built from different revisions.
 */
uint32_t poko_abi_version(void);

/**
 * @brief Creates an engine instance.
 *
 * @param config  Non-null, validated configuration.
 * @param out_engine  Non-null. Receives the handle on success and is set to
 *                    NULL on failure; never left dangling.
 * @return POKO_STATUS_OK, or a negative status. No exception escapes.
 */
int32_t poko_engine_create(const poko_engine_config* config, poko_engine** out_engine);

/**
 * @brief Advances the engine by one update.
 *
 * @param engine   Handle returned by poko_engine_create. Must not be NULL.
 * @param elapsed_seconds  Finite, non-negative time step in seconds.
 * @return POKO_STATUS_OK, or POKO_STATUS_INVALID_ARGUMENT for a NULL handle,
 *         a non-finite or negative step.
 */
int32_t poko_engine_update(poko_engine* engine, double elapsed_seconds);

/**
 * @brief Reads the number of updates applied so far.
 * @param engine Non-null handle.
 * @param out_frame_index Non-null; receives the counter.
 * @return POKO_STATUS_OK or a negative status.
 */
int32_t poko_engine_get_frame_index(const poko_engine* engine, uint64_t* out_frame_index);

/**
 * @brief Destroys an engine instance. Passing NULL is a no-op.
 *
 * The handle is invalid after this call; the caller must not use it again. The
 * library is not required to detect use-after-destroy.
 */
void poko_engine_destroy(poko_engine* engine);

/**
 * @brief Returns a static, permanently valid description of a status code.
 * @return Never NULL; unknown codes map to "unknown status".
 */
const char* poko_status_string(int32_t status);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* POKO_ENGINE_ABI_H */
