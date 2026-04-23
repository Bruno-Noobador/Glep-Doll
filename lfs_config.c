#include "lfs.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

#define FLASH_TARGET_OFFSET (1024 * 1024) // 1MB offset (adjust!)
#define FLASH_SIZE          (2 * 1024 * 1024) // Pico flash = 2MB

// --- READ ---
int lfs_read(const struct lfs_config *c, lfs_block_t block,
             lfs_off_t off, void *buffer, lfs_size_t size) {

    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size) + off;
    memcpy(buffer, (const void *)(XIP_BASE + addr), size);
    return 0;
}

// --- PROG ---
int lfs_prog(const struct lfs_config *c, lfs_block_t block,
             lfs_off_t off, const void *buffer, lfs_size_t size) {

    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size) + off;

    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(addr, buffer, size);
    restore_interrupts(ints);

    return 0;
}

// --- ERASE ---
int lfs_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(addr, c->block_size);
    restore_interrupts(ints);

    return 0;
}

// --- SYNC ---
int lfs_sync(const struct lfs_config *c) {
    return 0;
}