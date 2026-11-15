
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "esp_rom_sys.h" // Delay function
#include "led_strip.h"
#include "driver/uart.h"
#include "ble_uart.h"
#include "nvs_flash.h"

int green = 0;

char user_input[256];
bool new_data = false;

static void ble_rx_callback(const uint8_t *data, size_t len)
{
	new_data = true;
    printf("Received %u bytes\n", (unsigned)len);

	memcpy(user_input, data, len);

	for(int i = 0; i < len; i++)
		if(user_input[i] == '\n')
		{
			user_input[i] = '\0';
			break;
		}


    // Echo received data back to the laptop
    ble_uart_tx(data, len);
}

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
	uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);

    uart_param_config(UART_NUM_0, &uart_config);

    uart_set_pin(
        UART_NUM_0,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

	// BLE stuff
    // Initialize NVS (required by BLE)
    ESP_ERROR_CHECK(nvs_flash_init());

    // Configure BLE UART
    ble_uart_config_t ble_config = {
        .encrypted = false,
        .device_name = "OdysseyBoat",
        .ble_uart_on_rx = ble_rx_callback,
    };

    // Initialize and start BLE
    ESP_ERROR_CHECK(ble_uart_install(&ble_config));
    ESP_ERROR_CHECK(ble_uart_open());


	char* hello = "\nHello there!\n";
	uart_write_bytes(UART_NUM_0, hello, strlen(hello));

	// Main
	int led_value = 0;
	while(1)
	{
		//gpio_set_level(GPIO_NUM_8, led);
		//led_strip_set_pixel(led, 0, led_value, 0, 0);
		//led_strip_refresh(led);
		//led_value = ~led_value;
		//esp_rom_delay_us(500000);

		if(new_data)
		{
			new_data = false;
			if(!strcmp(user_input, "green"))
				green = ~green;
		}

		led_strip_set_pixel(led, 0, led_value, green, 0);
		led_strip_refresh(led);
	}
}
