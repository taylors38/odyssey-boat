
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash ble_uart esp_driver_uart esp_driver_gpio led_strip
)
