/* ============================================================================
 * PokoX M0-D — external C caller for the engine C ABI.
 *
 * Written in C11 and compiled as C, not C++: if any C++-only construct had
 * leaked into poko/engine_abi.h this file would not compile, which is exactly
 * the guarantee the ABI boundary needs.
 *
 * The static assertions pin the layout the caller expects. If a future change
 * silently grows, shrinks or reorders poko_engine_config, this translation unit
 * fails to build instead of corrupting memory at run time.
 * ==========================================================================*/

#include "poko/engine_abi.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

_Static_assert(sizeof(uint32_t) == 4, "the ABI assumes 32-bit uint32_t");
_Static_assert(sizeof(poko_engine_config) == 16, "poko_engine_config layout changed");
_Static_assert(offsetof(poko_engine_config, struct_size) == 0, "struct_size moved");
_Static_assert(offsetof(poko_engine_config, abi_version) == 4, "abi_version moved");
_Static_assert(offsetof(poko_engine_config, flags) == 8, "flags moved");
_Static_assert(offsetof(poko_engine_config, reserved0) == 12, "reserved0 moved");

static int g_failures = 0;

static void check(int condition, const char* what) {
    if (condition) {
        printf("  ok   : %s\n", what);
    } else {
        printf("  FAIL : %s\n", what);
        ++g_failures;
    }
}

static poko_engine_config make_config(void) {
    poko_engine_config config;
    config.struct_size = (uint32_t)sizeof(poko_engine_config);
    config.abi_version = POKO_ABI_VERSION;
    config.flags = POKO_ENGINE_FLAG_NONE;
    config.reserved0 = 0;
    return config;
}

int main(void) {
    printf("PokoX M0-D C ABI caller\n");
    printf("  abi version   : %u\n", poko_abi_version());
    printf("  config size   : %u bytes\n", (unsigned)sizeof(poko_engine_config));

    check(poko_abi_version() == POKO_ABI_VERSION, "library and header agree on ABI version");

    /* ---- happy path ----------------------------------------------------- */
    poko_engine_config config = make_config();
    poko_engine* engine = NULL;

    int32_t status = poko_engine_create(&config, &engine);
    check(status == POKO_STATUS_OK, "create succeeds with a valid config");
    check(engine != NULL, "create returns a non-null handle");

    for (int i = 0; i < 3; ++i) {
        status = poko_engine_update(engine, 0.016);
        check(status == POKO_STATUS_OK, "update succeeds");
    }

    uint64_t frame_index = 0;
    status = poko_engine_get_frame_index(engine, &frame_index);
    check(status == POKO_STATUS_OK, "get_frame_index succeeds");
    check(frame_index == 3, "frame index tracks the number of updates");

    poko_engine_destroy(engine);
    engine = NULL;

    /* ---- rejection of malformed input ----------------------------------- */
    poko_engine* rejected = (poko_engine*)0x1;
    status = poko_engine_create(NULL, &rejected);
    check(status == POKO_STATUS_INVALID_ARGUMENT, "create rejects a null config");
    check(rejected == NULL, "create clears the out handle on failure");

    rejected = (poko_engine*)0x1;
    status = poko_engine_create(&config, NULL);
    check(status == POKO_STATUS_INVALID_ARGUMENT, "create rejects a null out parameter");

    poko_engine_config too_small = make_config();
    too_small.struct_size = 8; /* older caller, fewer fields */
    status = poko_engine_create(&too_small, &rejected);
    check(status == POKO_STATUS_INVALID_CONFIG, "create rejects an undersized struct_size");

    poko_engine_config wrong_abi = make_config();
    wrong_abi.abi_version = POKO_ABI_VERSION + 1;
    status = poko_engine_create(&wrong_abi, &rejected);
    check(status == POKO_STATUS_ABI_MISMATCH, "create rejects an ABI version mismatch");

    poko_engine_config unknown_flag = make_config();
    unknown_flag.flags = 1u << 31;
    status = poko_engine_create(&unknown_flag, &rejected);
    check(status == POKO_STATUS_INVALID_CONFIG, "create rejects unknown flag bits");

    poko_engine_config dirty_reserved = make_config();
    dirty_reserved.reserved0 = 0xFFFF;
    status = poko_engine_create(&dirty_reserved, &rejected);
    check(status == POKO_STATUS_INVALID_CONFIG, "create rejects non-zero reserved fields");

    /* A larger struct from a newer caller must still be accepted. */
    struct future_config {
        poko_engine_config base;
        uint64_t future_field[2];
    } future;
    future.base = make_config();
    future.base.struct_size = (uint32_t)sizeof(future);
    future.future_field[0] = 0;
    future.future_field[1] = 0;
    status = poko_engine_create(&future.base, &rejected);
    check(status == POKO_STATUS_OK, "create accepts a larger struct from a newer caller");
    if (status == POKO_STATUS_OK) {
        poko_engine_destroy(rejected);
    }

    /* ---- defensive handling of null handles ----------------------------- */
    check(poko_engine_update(NULL, 0.016) == POKO_STATUS_INVALID_ARGUMENT,
          "update rejects a null handle");
    check(poko_engine_get_frame_index(NULL, &frame_index) == POKO_STATUS_INVALID_ARGUMENT,
          "get_frame_index rejects a null handle");
    check(poko_engine_get_frame_index(NULL, NULL) == POKO_STATUS_INVALID_ARGUMENT,
          "get_frame_index rejects two null arguments");
    poko_engine_destroy(NULL); /* must be a no-op */
    check(1, "destroy(NULL) is a no-op");

    /* ---- numeric validation --------------------------------------------- */
    engine = NULL;
    poko_engine_config numeric_config = make_config();
    status = poko_engine_create(&numeric_config, &engine);
    check(status == POKO_STATUS_OK, "create succeeds for numeric validation");
    if (status == POKO_STATUS_OK) {
        check(poko_engine_update(engine, NAN) == POKO_STATUS_INVALID_ARGUMENT,
              "update rejects NaN elapsed time");
        check(poko_engine_update(engine, INFINITY) == POKO_STATUS_INVALID_ARGUMENT,
              "update rejects infinite elapsed time");
        check(poko_engine_update(engine, -1.0) == POKO_STATUS_INVALID_ARGUMENT,
              "update rejects a negative elapsed time");
        poko_engine_destroy(engine);
    }

    /* ---- diagnostics ----------------------------------------------------- */
    check(poko_status_string(POKO_STATUS_ABI_MISMATCH) != NULL,
          "status strings are always available");
    check(poko_status_string(12345) != NULL, "unknown status codes still return a string");

    printf("%s\n", g_failures == 0 ? "PASS" : "FAILED");
    return g_failures == 0 ? 0 : 1;
}
