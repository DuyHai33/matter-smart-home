/* Minimal SSD1306 driver for the 0.96" 128x64 I2C module.
 *
 * In-tree rather than from the component registry: the app partition has about 7%
 * headroom left, and all this needs is text on eight lines.
 */

#pragma once

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGES (OLED_HEIGHT / 8)
#define OLED_COLS 21 /* characters per line with the 6-pixel font */
#define OLED_LINES OLED_PAGES

/** Bring up the I2C bus and the panel.
 *
 * @param sda        SDA pin
 * @param scl        SCL pin
 * @param i2c_addr   panel address, 0x3C on this module
 */
esp_err_t oled_init(int sda, int scl, uint8_t i2c_addr);

/** True once the panel answered on the bus */
bool oled_is_ready(void);

/** Clear the frame buffer (does not touch the panel until oled_flush) */
void oled_clear(void);

/** Draw a string into the frame buffer.
 *
 * @param line   text line, 0..OLED_LINES-1
 * @param col    character column, 0..OLED_COLS-1
 * @param text   ASCII string, clipped at the right edge
 */
void oled_text(uint8_t line, uint8_t col, const char *text);

/** Push the whole frame buffer to the panel */
esp_err_t oled_flush(void);

#ifdef __cplusplus
}
#endif
