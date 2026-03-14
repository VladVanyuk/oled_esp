#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "font8x8_basic.h"

#define TAG "SSD1306"

// -----------------------------
// User config
// -----------------------------
#define I2C_SCL_IO          9
#define I2C_SDA_IO          8
#define I2C_PORT_NUM        0
#define I2C_FREQ_HZ         400000

#define SSD1306_I2C_ADDR    0x3C
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGES          (OLED_HEIGHT / 8)

// -----------------------------
// SSD1306 state
// -----------------------------
static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t ssd1306_dev = NULL;

// 128 * 64 / 8 = 1024 bytes
static uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];


// -----------------------------
// Low-level SSD1306 functions
// -----------------------------
static esp_err_t ssd1306_write_command(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd}; // control byte 0x00 = command
    return i2c_master_transmit(ssd1306_dev, buf, sizeof(buf), -1);
}

static esp_err_t ssd1306_write_data(const uint8_t *data, size_t len)
{
    // prepend control byte 0x40
    uint8_t tx[17];
    tx[0] = 0x40;

    while (len > 0) {
        size_t chunk = (len > 16) ? 16 : len;
        memcpy(&tx[1], data, chunk);
        esp_err_t err = i2c_master_transmit(ssd1306_dev, tx, chunk + 1, -1);
        if (err != ESP_OK) {
            return err;
        }
        data += chunk;
        len -= chunk;
    }
    return ESP_OK;
}

static esp_err_t i2c_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags.enable_internal_pullup = true,
    };

    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &i2c_bus), TAG, "i2c_new_master_bus failed");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_I2C_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &ssd1306_dev), TAG, "add_device failed");
    return ESP_OK;
}

static esp_err_t ssd1306_init(void)
{
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xAE), TAG, "display off");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x20), TAG, "memory mode");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x00), TAG, "horizontal addressing mode");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xB0), TAG, "page start");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xC8), TAG, "COM scan dec");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x00), TAG, "low column");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x10), TAG, "high column");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x40), TAG, "start line");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x81), TAG, "contrast");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x7F), TAG, "contrast value");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xA1), TAG, "segment remap");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xA6), TAG, "normal display");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xA8), TAG, "multiplex");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x3F), TAG, "1/64 duty");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xA4), TAG, "display follows RAM");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xD3), TAG, "display offset");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x00), TAG, "no offset");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xD5), TAG, "clock divide");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x80), TAG, "clock divide value");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xD9), TAG, "pre-charge");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xF1), TAG, "pre-charge value");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xDA), TAG, "com pins");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x12), TAG, "com pins value");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xDB), TAG, "vcom detect");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x40), TAG, "vcom value");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x8D), TAG, "charge pump");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x14), TAG, "enable charge pump");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0xAF), TAG, "display on");
    return ESP_OK;
}

// -----------------------------
// Graphics helpers
// -----------------------------
static void oled_clear(void)
{
    memset(oled_buffer, 0x00, sizeof(oled_buffer));
}

static void oled_set_pixel(int x, int y, bool color)
{
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
        return;
    }

    uint16_t index = x + (y / 8 * OLED_WIDTH);
    uint8_t mask = 1 << (y % 8);

    if (color) {
        oled_buffer[index] |= mask;
    } else {
        oled_buffer[index] &= ~mask;
    }
}

static void oled_draw_char(int x, int y, char c)
{
    if (c < 32 || c > 127) {
        c = '?';
    }
    const uint8_t *glyph = font5x7[c - 32];

    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 7; row++) {
            bool pixel_on = (line >> row) & 0x01;
            oled_set_pixel(x + col, y + row, pixel_on);
        }
    }

    // 1 column spacing
    for (int row = 0; row < 7; row++) {
        oled_set_pixel(x + 5, y + row, false);
    }
}

static void oled_draw_string(int x, int y, const char *str)
{
    while (*str) {
        oled_draw_char(x, y, *str++);
        x += 6;
        if (x > (OLED_WIDTH - 6)) {
            break;
        }
    }
}

static esp_err_t oled_update(void)
{
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x21), TAG, "set column addr");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x00), TAG, "column start");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(OLED_WIDTH - 1), TAG, "column end");

    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x22), TAG, "set page addr");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(0x00), TAG, "page start");
    ESP_RETURN_ON_ERROR(ssd1306_write_command(OLED_PAGES - 1), TAG, "page end");

    return ssd1306_write_data(oled_buffer, sizeof(oled_buffer));
}

static void oled_draw_hline(int x, int y, int w, bool color)
{
    for (int i = 0; i < w; i++) {
        oled_set_pixel(x + i, y, color);
    }
}

// -----------------------------
// FreeRTOS display task
// -----------------------------
static void oled_task(void *arg)
{
    int counter = 0;

    while (1) {
        char line1[32];
        char line2[32];
        char line3[32];

        snprintf(line1, sizeof(line1), "ESP32-S3 + OLED");
        snprintf(line2, sizeof(line2), "SSD1306 / ESP-IDF");
        snprintf(line3, sizeof(line3), "Count: %d", counter++);

        oled_clear();
        oled_draw_string(0, 0, line1);
        oled_draw_string(0, 16, line2);
        oled_draw_hline(0, 30, 128, true);
        oled_draw_string(0, 40, line3);

        esp_err_t err = oled_update();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "oled_update failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(i2c_init());
    ESP_ERROR_CHECK(ssd1306_init());

    oled_clear();
    oled_draw_string(0, 0, "Booting...");
    ESP_ERROR_CHECK(oled_update());

    xTaskCreate(oled_task, "oled_task", 4096, NULL, 5, NULL);
}