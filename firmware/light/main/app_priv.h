/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include <driver/gpio.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

/** Standard max values (used for remapping attributes) */
#define STANDARD_BRIGHTNESS 100
#define STANDARD_HUE 360
#define STANDARD_SATURATION 100
#define STANDARD_TEMPERATURE_FACTOR 1000000

/** Matter max values (used for remapping attributes) */
#define MATTER_BRIGHTNESS 254
#define MATTER_HUE 254
#define MATTER_SATURATION 254
#define MATTER_TEMPERATURE_FACTOR 1000000

/** Default attribute values used during initialization */
#define DEFAULT_POWER true
#define DEFAULT_BRIGHTNESS 64
#define DEFAULT_HUE 128
#define DEFAULT_SATURATION 254

/** Indicator LEDs on the breadboard. Plain single-colour LEDs on GPIO outputs, not
 * the light of endpoint 1 - that stays on the onboard WS2812 at GPIO8. */
#define LED_NETWORK_GPIO GPIO_NUM_0 /* green  - Wi-Fi up and commissioned */
#define LED_ONOFF_GPIO   GPIO_NUM_1 /* red    - mirrors the OnOff state of endpoint 1 */
#define LED_COMMAND_GPIO GPIO_NUM_2 /* blue   - blinks once per incoming Matter command */

/** Push buttons, wired to ground with the internal pull-up (pressed = low) */
#define BUTTON_TOGGLE_GPIO GPIO_NUM_20 /* toggles OnOff of endpoint 1 */
#define BUTTON_SWITCH_GPIO GPIO_NUM_19 /* generic switch on endpoint 2 */
#define BUTTON_PAGE_GPIO   GPIO_NUM_18 /* cycles the OLED page */

/** How long the blue LED stays lit for a single command, in milliseconds */
#define LED_COMMAND_BLINK_MS 80

typedef void *app_driver_handle_t;

/** Configure the three indicator LEDs as outputs and turn them off */
esp_err_t app_indicator_init();

/** Green LED: network state */
void app_indicator_set_network(bool up);

/** Red LED: OnOff state of endpoint 1 */
void app_indicator_set_onoff(bool on);

/** Blink the blue LED once. Arms a timer, so it is safe from a Matter callback. */
void app_indicator_blink_command();

/** Count one Matter attribute update. From the attribute callback only. */
void app_count_attribute_update();

/** Initialize the three breadboard buttons.
 *
 * Separate from app_driver_button_init(), which owns the onboard BOOT button and the
 * factory-reset long press. None of these three can reset the device.
 */
esp_err_t app_driver_buttons_init();

/** Currently displayed OLED page, cycled by the page button */
uint8_t app_get_oled_page();

/** Matter attribute updates seen since boot */
uint32_t app_get_command_count();

/** Number of presses of a given button: 0 = toggle, 1 = switch, 2 = page */
uint32_t app_get_button_count(uint8_t index);

/** OLED on I2C. The C6 routes I2C through the GPIO matrix so any pin works; 6 and 7
 * sit on the 2.54 mm header, while GPIO21-23 are bare pads on the board edge. */
#define OLED_SDA_GPIO 6
#define OLED_SCL_GPIO 7
#define OLED_I2C_ADDR 0x3C

/** Bring up the OLED. Returns an error if the panel does not answer on the bus. */
esp_err_t app_oled_init();

/** Redraw the current page. Call from a normal task, not from a Matter callback. */
void app_oled_update();

/** Initialize the light driver
 *
 * This initializes the light driver associated with the selected board.
 *
 * @return Handle on success.
 * @return NULL in case of failure.
 */
app_driver_handle_t app_driver_light_init();

/** Initialize the button driver
 *
 * This initializes the button driver associated with the selected board.
 *
 * @return Handle on success.
 * @return NULL in case of failure.
 */
app_driver_handle_t app_driver_button_init();

/** Driver Update
 *
 * This API should be called to update the driver for the attribute being updated.
 * This is usually called from the common `app_attribute_update_cb()`.
 *
 * @param[in] endpoint_id Endpoint ID of the attribute.
 * @param[in] cluster_id Cluster ID of the attribute.
 * @param[in] attribute_id Attribute ID of the attribute.
 * @param[in] val Pointer to `esp_matter_attr_val_t`. Use appropriate elements as per the value type.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);

/** Set defaults for light driver
 *
 * Set the attribute drivers to their default values from the created data model.
 *
 * @param[in] endpoint_id Endpoint ID of the driver.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif
