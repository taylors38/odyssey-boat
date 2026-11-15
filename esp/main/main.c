
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "esp_rom_sys.h" // Delay function
#include "led_strip.h"
#include "driver/uart.h"
#include "ble_uart.h"
#include "nvs_flash.h"
#include "pwm.h"

int green = 0;
int red = 0;

char user_input[256];
bool new_data = false;

int n_user_args;
char* user_args[4];

static void ble_rx_callback(const uint8_t *data, size_t len)
{
	new_data = true;
	n_user_args = 0;
    printf("Received %u bytes\n", (unsigned)len);

	memcpy(user_input, data, len);

	for(int i = 0; i < len; i++)
	{
		if(user_input[i] == '\n')
		{
			user_input[i] = '\0';
			break;
		}

		if(user_input[i] == ' ')
		{
			user_input[i] = '\0';
			if(n_user_args >= 4 || i == (len - 1))
				return;

			user_args[n_user_args++] = &user_input[i+1];
		}
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


	/* GPIO Stuff */
	gpio_set_direction(GPIO_NUM_10, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_11, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_14, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);

	pwm_init();

	char* hello = "\nHello there!\n";
	uart_write_bytes(UART_NUM_0, hello, strlen(hello));

	// Main
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

			if(!strcmp(user_input, "row"))
			{
				printf("rowing\n");
				red = ~red;
				gpio_set_level(GPIO_NUM_10, 1);
				gpio_set_level(GPIO_NUM_11, 1);
				gpio_set_level(GPIO_NUM_12, 1);
				gpio_set_level(GPIO_NUM_13, 1);
				gpio_set_level(GPIO_NUM_14, 1);
				gpio_set_level(GPIO_NUM_15, 1);
			}

			if(!strcmp(user_input, "stop"))
			{
				printf("stopping\n");
				red = 0;
				gpio_set_level(GPIO_NUM_10, 0);
				gpio_set_level(GPIO_NUM_11, 0);
				gpio_set_level(GPIO_NUM_12, 0);
				gpio_set_level(GPIO_NUM_13, 0);
				gpio_set_level(GPIO_NUM_14, 0);
				gpio_set_level(GPIO_NUM_15, 0);
			}

			if(!strcmp(user_input, "left"))
			{
				
				if(n_user_args > 1)
				{
					float f = strtof(user_args[1], NULL);
					pwm_set_duty(0, f);
					pwm_set_duty(1, f);
					pwm_set_duty(2, f);
				}
			}

			if(!strcmp(user_input, "right"))
			{
				
				if(n_user_args > 1)
				{
					float f = strtof(user_args[1], NULL);
					pwm_set_duty(3, f);
					pwm_set_duty(4, f);
					pwm_set_duty(5, f);
				}
			}
		}

		led_strip_set_pixel(led, 0, red, green, 0);
		led_strip_refresh(led);
	}
}
