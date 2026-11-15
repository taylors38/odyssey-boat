
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "esp_rom_sys.h" // Delay function
#include "led_strip.h"
#include "driver/uart.h"

int app_main()
{
	//gpio_set_direction(GPIO_NUM_8, GPIO_MODE_OUTPUT);

	// RGB LED stuff
	led_strip_config_t strip_config = {
		.strip_gpio_num = GPIO_NUM_8,
		.max_leds = 1,
		.led_model = LED_MODEL_WS2812,
		.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB
	};

	led_strip_rmt_config_t rmt_config =  {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000,
        .mem_block_symbols = 64,
    };

	led_strip_handle_t led;
	led_strip_new_rmt_device(&strip_config, &rmt_config, &led);

	// UART Stuff
	uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);

    uart_param_config(UART_NUM_0, &config);

    uart_set_pin(
        UART_NUM_0,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

	char* hello = "Hello there!\n";
	uart_write_bytes(UART_NUM_0, hello, strlen(hello));

	// Main
	int led_value = 0;
	while(1)
	{
		//gpio_set_level(GPIO_NUM_8, led);
		led_strip_set_pixel(led, 0, led_value, 0, 0);
		led_strip_refresh(led);
		led_value = ~led_value;
		esp_rom_delay_us(500000);
	}
}
