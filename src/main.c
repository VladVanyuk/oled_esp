#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "freertos/queue.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "driver/i2c.h"

#include "ssd1306.h"

#define SDA_PIN 21
#define SCL_PIN 22
#define PIXEL_CLOCK_HZ    (400 * 1000)
#define CONFIG_RESET_GPIO 8


void display_task(void *pvParameter)
{
    i2c_config_t *i2c_conf = (i2c_config_t *)pvParameter;

    //  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, i2c_conf));
    //  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    SSD1306_t dev;
    dev._address = I2C_ADDRESS;
    i2c_master_init(&dev, i2c_conf->sda_io_num, i2c_conf->scl_io_num, CONFIG_RESET_GPIO);

    ssd1306_init(&dev, 128, 64);
    uint32_t count = 0;
    while(1) {
        char text[32];
        snprintf(text, sizeof(text), "Count: %d", count);

        ssd1306_display_text(&dev, 0, text, sizeof(text), false);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        count++;

        //
    }
}


void sensor_task(void *pvParameter)
{
    while(1) {
        ESP_LOGI(__FUNCTION__, "Sensor task is running");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main() {

    ESP_LOGI(__FUNCTION__, "Init started");

     i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = PIXEL_CLOCK_HZ,
    };

   // ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
   //  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    xTaskCreate(display_task, "display_task", 4096, &i2c_conf, 5, NULL);
    xTaskCreate(sensor_task, "sensor_task", (4096/2), &i2c_conf, 4, NULL);
    
}