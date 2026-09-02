/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <esp_matter.h>
#include <app_priv.h>
#include <common_macros.h>

#include <device.h>
#include <led_driver.h>
#include <button_gpio.h>
#include <iot_button.h>
#include <esp_timer.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;
extern uint16_t switch_endpoint_id;

// Global variables to store current XY color coordinates
static uint16_t current_x = 0;
static uint16_t current_y = 0;

/* ------------------------------------------------------------------ indicator LEDs */

static esp_timer_handle_t s_blink_timer = NULL;

/* volatile: written from the button and Matter tasks, read from the display loop */
static volatile uint8_t s_oled_page = 0;
static volatile uint32_t s_command_count = 0;
static volatile uint32_t s_button_count[3] = {0, 0, 0};

uint32_t app_get_command_count()
{
    return s_command_count;
}

uint32_t app_get_button_count(uint8_t index)
{
    return (index < 3) ? s_button_count[index] : 0;
}

static void app_indicator_blink_off_cb(void *arg)
{
    gpio_set_level(LED_COMMAND_GPIO, 0);
}

esp_err_t app_indicator_init()
{
    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL << LED_NETWORK_GPIO) | (1ULL << LED_ONOFF_GPIO) | (1ULL << LED_COMMAND_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&out_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure indicator LEDs: %d", err);
        return err;
    }

    gpio_set_level(LED_NETWORK_GPIO, 0);
    gpio_set_level(LED_ONOFF_GPIO, 0);
    gpio_set_level(LED_COMMAND_GPIO, 0);

    /* One-shot, so the blink can be armed from a callback without blocking it */
    const esp_timer_create_args_t timer_args = {
        .callback = app_indicator_blink_off_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "led_blink_off",
        .skip_unhandled_events = true,
    };
    err = esp_timer_create(&timer_args, &s_blink_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create blink timer: %d", err);
        return err;
    }

    ESP_LOGI(TAG, "Indicator LEDs ready: green=GPIO%d red=GPIO%d blue=GPIO%d",
             LED_NETWORK_GPIO, LED_ONOFF_GPIO, LED_COMMAND_GPIO);
    return ESP_OK;
}

void app_indicator_set_network(bool up)
{
    gpio_set_level(LED_NETWORK_GPIO, up ? 1 : 0);
}

void app_indicator_set_onoff(bool on)
{
    gpio_set_level(LED_ONOFF_GPIO, on ? 1 : 0);
}

void app_indicator_blink_command()
{
    if (s_blink_timer == NULL) {
        return;
    }
    gpio_set_level(LED_COMMAND_GPIO, 1);
    /* Stop first, so back-to-back commands extend the blink */
    esp_timer_stop(s_blink_timer);
    esp_timer_start_once(s_blink_timer, LED_COMMAND_BLINK_MS * 1000);
}

void app_count_attribute_update()
{
    s_command_count++;
}

uint8_t app_get_oled_page()
{
    return s_oled_page;
}

/* Do any conversions/remapping for the actual value here */
static esp_err_t app_driver_light_set_power(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    /* Red LED follows the light, whatever changed it */
    app_indicator_set_onoff(val->val.b);
    return led_driver_set_power(handle, val->val.b);
}

static esp_err_t app_driver_light_set_brightness(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_BRIGHTNESS, STANDARD_BRIGHTNESS);
    return led_driver_set_brightness(handle, value);
}

static esp_err_t app_driver_light_set_hue(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_HUE, STANDARD_HUE);
    return led_driver_set_hue(handle, value);
}

static esp_err_t app_driver_light_set_saturation(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_SATURATION, STANDARD_SATURATION);
    return led_driver_set_saturation(handle, value);
}

static esp_err_t app_driver_light_set_temperature(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    uint32_t value = REMAP_TO_RANGE_INVERSE(val->val.u16, STANDARD_TEMPERATURE_FACTOR);
    return led_driver_set_temperature(handle, value);
}

static esp_err_t app_driver_light_set_xy(led_driver_handle_t handle, uint16_t x, uint16_t y)
{
    return led_driver_set_xy(handle, x, y);
}

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);

    esp_matter_attr_val_t val;
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

/* ------------------------------------------------------------------ breadboard buttons */

/* Button 1 (GPIO20): toggle EP1, same path as the onboard button */
static void app_button_toggle_cb(void *arg, void *data)
{
    s_button_count[0]++;
    ESP_LOGI(TAG, "Button 1: toggle light");
    app_driver_button_toggle_cb(arg, data);
}

/* Button 2 (GPIO19): generic switch on EP2. Reported as events, not attributes.
 * The event helpers call chipDie() if used outside the Matter task, and button
 * callbacks run in the button task - hence ScheduleWork. */
static void app_switch_send_press(intptr_t arg)
{
    cluster::switch_cluster::event::send_initial_press(switch_endpoint_id, 1);
}

static void app_switch_send_release(intptr_t arg)
{
    cluster::switch_cluster::event::send_short_release(switch_endpoint_id, 1);
}

/* ScheduleWork fails if the Matter queue is full. Don't drop a press silently. */
static void app_switch_schedule(chip::DeviceLayer::AsyncWorkFunct work, const char *what)
{
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(work, 0);
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "Dropped %s: %" CHIP_ERROR_FORMAT, what, err.Format());
    }
}

static void app_button_switch_press_cb(void *arg, void *data)
{
    if (switch_endpoint_id == 0) {
        return;
    }
    s_button_count[1]++;
    ESP_LOGI(TAG, "Button 2: InitialPress on endpoint %d", switch_endpoint_id);
    app_switch_schedule(app_switch_send_press, "InitialPress");
    app_indicator_blink_command(); /* blink only - a local press is not a Matter command */
}

static void app_button_switch_release_cb(void *arg, void *data)
{
    if (switch_endpoint_id == 0) {
        return;
    }
    ESP_LOGI(TAG, "Button 2: ShortRelease on endpoint %d", switch_endpoint_id);
    app_switch_schedule(app_switch_send_release, "ShortRelease");
}

/* Button 3 (GPIO18): cycle the OLED page. Purely local, touches no cluster. */
static void app_button_page_cb(void *arg, void *data)
{
    s_button_count[2]++;
    s_oled_page = (s_oled_page + 1) % 3;
    ESP_LOGI(TAG, "Button 3: OLED page -> %d", s_oled_page);
}

/* The switch button needs two callbacks on one handle, so return it */
static button_handle_t app_button_create(gpio_num_t gpio, const char *name)
{
    button_handle_t handle = NULL;
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t gpio_cfg = {
        .gpio_num = gpio,
        .active_level = 0, /* wired to ground, pressed = low */
        .enable_power_save = false,
        .disable_pull = false, /* internal pull-up, no external resistor needed */
    };

    esp_err_t err = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button %s on GPIO%d: %d", name, gpio, err);
        return NULL;
    }
    return handle;
}

esp_err_t app_driver_buttons_init()
{
    button_handle_t toggle_btn = app_button_create(BUTTON_TOGGLE_GPIO, "toggle");
    button_handle_t switch_btn = app_button_create(BUTTON_SWITCH_GPIO, "switch");
    button_handle_t page_btn = app_button_create(BUTTON_PAGE_GPIO, "page");
    esp_err_t err = ESP_FAIL;

    if (toggle_btn == NULL || switch_btn == NULL || page_btn == NULL) {
        goto fail;
    }

    /* One at a time - OR-ing esp_err_t codes together gives a meaningless value */
    err = iot_button_register_cb(toggle_btn, BUTTON_PRESS_DOWN, NULL, app_button_toggle_cb, NULL);
    if (err != ESP_OK) {
        goto fail;
    }
    err = iot_button_register_cb(switch_btn, BUTTON_PRESS_DOWN, NULL, app_button_switch_press_cb, NULL);
    if (err != ESP_OK) {
        goto fail;
    }
    err = iot_button_register_cb(switch_btn, BUTTON_PRESS_UP, NULL, app_button_switch_release_cb, NULL);
    if (err != ESP_OK) {
        goto fail;
    }
    err = iot_button_register_cb(page_btn, BUTTON_PRESS_DOWN, NULL, app_button_page_cb, NULL);
    if (err != ESP_OK) {
        goto fail;
    }

    ESP_LOGI(TAG, "Buttons ready: toggle=GPIO%d switch=GPIO%d page=GPIO%d",
             BUTTON_TOGGLE_GPIO, BUTTON_SWITCH_GPIO, BUTTON_PAGE_GPIO);
    return ESP_OK;

fail:
    ESP_LOGE(TAG, "Button init failed: %s", esp_err_to_name(err));
    /* Delete what was created, or the GPIOs stay claimed */
    if (toggle_btn) {
        iot_button_delete(toggle_btn);
    }
    if (switch_btn) {
        iot_button_delete(switch_btn);
    }
    if (page_btn) {
        iot_button_delete(page_btn);
    }
    return err;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_endpoint_id) {
        led_driver_handle_t handle = (led_driver_handle_t)driver_handle;
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                err = app_driver_light_set_power(handle, val);
            }
        } else if (cluster_id == LevelControl::Id) {
            if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
                err = app_driver_light_set_brightness(handle, val);
            }
        } else if (cluster_id == ColorControl::Id) {
            if (attribute_id == ColorControl::Attributes::CurrentHue::Id) {
                err = app_driver_light_set_hue(handle, val);
            } else if (attribute_id == ColorControl::Attributes::CurrentSaturation::Id) {
                err = app_driver_light_set_saturation(handle, val);
            } else if (attribute_id == ColorControl::Attributes::ColorTemperatureMireds::Id) {
                err = app_driver_light_set_temperature(handle, val);
            } else if (attribute_id == ColorControl::Attributes::CurrentX::Id) {
                current_x = val->val.u16;
                err = app_driver_light_set_xy(handle, current_x, current_y);
            } else if (attribute_id == ColorControl::Attributes::CurrentY::Id) {
                current_y = val->val.u16;
                err = app_driver_light_set_xy(handle, current_x, current_y);
            }
        }
    }
    return err;
}

esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    void *priv_data = endpoint::get_priv_data(endpoint_id);
    led_driver_handle_t handle = (led_driver_handle_t)priv_data;
    esp_matter_attr_val_t val;

    /* Setting brightness */
    attribute_t *attribute = attribute::get(endpoint_id, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_brightness(handle, &val);

    /* Setting color */
    attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::ColorMode::Id);
    attribute::get_val(attribute, &val);
    if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kCurrentHueAndCurrentSaturation) {
        /* Setting hue */
        attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::CurrentHue::Id);
        attribute::get_val(attribute, &val);
        err |= app_driver_light_set_hue(handle, &val);
        /* Setting saturation */
        attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::CurrentSaturation::Id);
        attribute::get_val(attribute, &val);
        err |= app_driver_light_set_saturation(handle, &val);
    } else if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kColorTemperature) {
        /* Setting temperature */
        attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id);
        attribute::get_val(attribute, &val);
        err |= app_driver_light_set_temperature(handle, &val);
    } else if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kCurrentXAndCurrentY) {
        /* Setting XY coordinates */
        attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::CurrentX::Id);
        attribute::get_val(attribute, &val);
        current_x = val.val.u16;
        attribute = attribute::get(endpoint_id, ColorControl::Id, ColorControl::Attributes::CurrentY::Id);
        attribute::get_val(attribute, &val);
        current_y = val.val.u16;
        err |= app_driver_light_set_xy(handle, current_x, current_y);
    } else {
        ESP_LOGE(TAG, "Color mode not supported");
    }

    /* Setting power */
    attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_power(handle, &val);

    return err;
}

app_driver_handle_t app_driver_light_init()
{
    /* Initialize led */
    led_driver_config_t config = led_driver_get_config();
    led_driver_handle_t handle = led_driver_init(&config);
    return (app_driver_handle_t)handle;
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_handle_t handle = NULL;
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t btn_gpio_cfg = button_driver_get_config();

    if (iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button device");
        return NULL;
    }

    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, NULL, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}
