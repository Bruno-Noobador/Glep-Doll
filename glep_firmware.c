#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
//#include "pico/sleep.h"

#include "lfs.h"

// ================== CONFIG ==================
#define AUDIO_PIN 15
#define SHAKE_PIN 2

#define SAMPLE_RATE 22050
#define SHAKE_TIME_MS 2000

#define FLASH_TARGET_OFFSET (1024 * 1024)

// ================== WAV HEADER ==================
typedef struct {
    char riff[4];
    uint32_t size;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t format;
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
} WAVHeader;

// ================== LITTLEFS ==================
lfs_t lfs;
struct lfs_config cfg;

// --- low-level flash ops ---
int lfs_read(const struct lfs_config *c, lfs_block_t block,
             lfs_off_t off, void *buffer, lfs_size_t size) {

    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size) + off;
    memcpy(buffer, (const void *)(XIP_BASE + addr), size);
    return 0;
}

int lfs_prog(const struct lfs_config *c, lfs_block_t block,
             lfs_off_t off, const void *buffer, lfs_size_t size) {

    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size) + off;

    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(addr, buffer, size);
    restore_interrupts(ints);

    return 0;
}

int lfs_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t addr = FLASH_TARGET_OFFSET + (block * c->block_size);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(addr, c->block_size);
    restore_interrupts(ints);

    return 0;
}

int lfs_sync(const struct lfs_config *c) {
    return 0;
}

// --- init FS ---
void init_lfs() {
    cfg.read  = lfs_read;
    cfg.prog  = lfs_prog;
    cfg.erase = lfs_erase;
    cfg.sync  = lfs_sync;

    cfg.read_size = 256;
    cfg.prog_size = 256;
    cfg.block_size = 4096;
    cfg.block_count = 256;
    cfg.cache_size = 256;
    cfg.lookahead_size = 16;
    cfg.block_cycles = 500;

    int err = lfs_mount(&lfs, &cfg);

    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }
}

// ================== PWM AUDIO ==================
uint slice_num;

void setup_pwm() {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_config config = pwm_get_default_config();

    float clkdiv = (float)clock_get_hz(clk_sys) / (SAMPLE_RATE * 65536);
    pwm_config_set_clkdiv(&config, clkdiv);

    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(AUDIO_PIN, 32768);
}

static inline void output_sample(int16_t sample) {
    uint16_t pwm = sample + 32768;
    pwm_set_gpio_level(AUDIO_PIN, pwm);
}

// ================== WAV PLAYER ==================
void play_wav_file(const char *filename) {
    lfs_file_t file;

    if (lfs_file_open(&lfs, &file, filename, LFS_O_RDONLY) < 0) {
        printf("Failed to open %s\n", filename);
        return;
    }

    WAVHeader header;

    if (lfs_file_read(&lfs, &file, &header, sizeof(header)) != sizeof(header)) {
        printf("Invalid header\n");
        lfs_file_close(&lfs, &file);
        return;
    }

    if (header.sampleRate != SAMPLE_RATE ||
        header.bitsPerSample != 16 ||
        header.channels != 1 ||
        header.format != 1) {

        printf("Unsupported WAV format\n");
        lfs_file_close(&lfs, &file);
        return;
    }

    uint8_t buffer[512];
    int bytesRead;

    uint32_t delay_us = 1000000UL / SAMPLE_RATE;

    while ((bytesRead = lfs_file_read(&lfs, &file, buffer, sizeof(buffer))) > 0) {

        for (int i = 0; i < bytesRead; i += 2) {
            int16_t sample = buffer[i] | (buffer[i + 1] << 8);
            output_sample(sample);
            //sleep_us(delay_us);
        }
    }

    pwm_set_gpio_level(AUDIO_PIN, 32768);

    lfs_file_close(&lfs, &file);
}

// ================== RANDOM PLAY ==================
void play_random_glep() {
    // Simple version (you can extend to directory scan)
    play_wav_file("glep1.wav");
}

// ================== DORMANT ==================
void go_dormant() {
    gpio_pull_down(SHAKE_PIN);

    //sleep_run_from_xosc();

    gpio_set_dormant_irq_enabled(SHAKE_PIN, GPIO_IRQ_EDGE_RISE, true);

    //sleep_goto_dormant();
}

// ================== MAIN ==================
int main() {
    stdio_init_all();

    gpio_init(SHAKE_PIN);
    gpio_set_dir(SHAKE_PIN, GPIO_IN);

    setup_pwm();
    init_lfs();

    printf("Glep firmware started\n");

    absolute_time_t shakeStart;
    bool shaking = false;
    bool alreadyPlayed = false;

    while (true) {
        bool shakeNow = gpio_get(SHAKE_PIN);

        if (shakeNow) {
            if (!shaking) {
                shaking = true;
                shakeStart = get_absolute_time();
            } else {
                int64_t elapsedMs =
                    absolute_time_diff_us(shakeStart, get_absolute_time()) / 1000;

                if (elapsedMs >= SHAKE_TIME_MS && !alreadyPlayed) {
                    alreadyPlayed = true;

                    play_random_glep();

                    go_dormant();
                }
            }
        } else {
            shaking = false;
            alreadyPlayed = false;
        }

        //sleep_ms(10);
    }
}