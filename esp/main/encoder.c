#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"

#define ENCODER_A GPIO_NUM_20
#define ENCODER_B GPIO_NUM_21

#define PCNT_HIGH_LIMIT  30000
#define PCNT_LOW_LIMIT  -30000

static const char *TAG = "ENCODER";

static pcnt_unit_handle_t encoder_unit = NULL;

void encoder_init(void)
{
    // Configure GPIO pull-ups
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << ENCODER_A) | (1ULL << ENCODER_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Configure PCNT unit
    pcnt_unit_config_t unit_config = {
        .high_limit = PCNT_HIGH_LIMIT,
        .low_limit = PCNT_LOW_LIMIT,
    };

    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &encoder_unit));

    // Configure glitch filter
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };

    ESP_ERROR_CHECK(
        pcnt_unit_set_glitch_filter(encoder_unit, &filter_config)
    );

    // Channel 0: A is edge, B is level
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = ENCODER_A,
        .level_gpio_num = ENCODER_B,
    };

    pcnt_channel_handle_t chan_a = NULL;

    ESP_ERROR_CHECK(
        pcnt_new_channel(encoder_unit, &chan_a_config, &chan_a)
    );

    // Channel 1: B is edge, A is level
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = ENCODER_B,
        .level_gpio_num = ENCODER_A,
    };

    pcnt_channel_handle_t chan_b = NULL;

    ESP_ERROR_CHECK(
        pcnt_new_channel(encoder_unit, &chan_b_config, &chan_b)
    );

    // Configure channel A
    ESP_ERROR_CHECK(
        pcnt_channel_set_edge_action(
            chan_a,
            PCNT_CHANNEL_EDGE_ACTION_DECREASE,
            PCNT_CHANNEL_EDGE_ACTION_INCREASE
        )
    );

    ESP_ERROR_CHECK(
        pcnt_channel_set_level_action(
            chan_a,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE
        )
    );

    // Configure channel B
    ESP_ERROR_CHECK(
        pcnt_channel_set_edge_action(
            chan_b,
            PCNT_CHANNEL_EDGE_ACTION_INCREASE,
            PCNT_CHANNEL_EDGE_ACTION_DECREASE
        )
    );

    ESP_ERROR_CHECK(
        pcnt_channel_set_level_action(
            chan_b,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE
        )
    );

    // Enable and start PCNT
    ESP_ERROR_CHECK(pcnt_unit_enable(encoder_unit));

    ESP_ERROR_CHECK(pcnt_unit_clear_count(encoder_unit));

    ESP_ERROR_CHECK(pcnt_unit_start(encoder_unit));

    ESP_LOGI(TAG, "Quadrature encoder initialized");
}

int encoder_get_count(void)
{
    int count = 0;

    ESP_ERROR_CHECK(
        pcnt_unit_get_count(encoder_unit, &count)
    );

    return count;
}

void encoder_reset(void)
{
    ESP_ERROR_CHECK(
        pcnt_unit_clear_count(encoder_unit)
    );
}
