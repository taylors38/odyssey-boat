
idf_component_register(
    SRCS "main.c"
		 "pwm.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_mcpwm nvs_flash ble_uart esp_driver_uart esp_driver_gpio led_strip
)
