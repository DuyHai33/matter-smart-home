/* OLED status screen. Three pages, cycled by the button on GPIO18:
 *   0 - network and light state, plus the fabric count
 *   1 - one line per commissioned fabric
 *   2 - counters
 */

#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_matter.h>
#include <esp_netif.h>
#include <esp_timer.h>
#include <esp_wifi.h>

#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

#include <app_priv.h>
#include "oled_ssd1306.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_oled";

extern uint16_t light_endpoint_id;

esp_err_t app_oled_init()
{
    return oled_init(OLED_SDA_GPIO, OLED_SCL_GPIO, OLED_I2C_ADDR);
}

/* SSID trimmed to what fits, plus RSSI. Wi-Fi driver call, no stack lock needed. */
static void app_oled_wifi_line(char *out, size_t len)
{
    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) {
        snprintf(out, len, "WiFi: khong ket noi");
        return;
    }
    /* SSID can be 32 bytes, only about a dozen fit next to the RSSI */
    char ssid[13];
    size_t n = strnlen((const char *)ap.ssid, sizeof(ssid) - 1);
    memcpy(ssid, ap.ssid, n);
    ssid[n] = '\0';

    snprintf(out, len, "%s %ddBm", ssid, ap.rssi);
}

static void app_oled_ip_line(char *out, size_t len)
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip;
    if (netif == NULL || esp_netif_get_ip_info(netif, &ip) != ESP_OK || ip.ip.addr == 0) {
        snprintf(out, len, "IP: ---");
        return;
    }
    snprintf(out, len, "IP:" IPSTR, IP2STR(&ip.ip));
}

/* Page 0 - what the device is doing right now */
static void app_oled_draw_status()
{
    char line[OLED_COLS + 1];

    oled_text(0, 0, "MATTER DEVICE");

    app_oled_wifi_line(line, sizeof(line));
    oled_text(1, 0, line);

    app_oled_ip_line(line, sizeof(line));
    oled_text(2, 0, line);

    /* Read under the lock - touching the stack without it makes CHIP abort */
    uint8_t fabrics = 0;
    bool on = false;
    uint8_t level = 0;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    fabrics = chip::Server::GetInstance().GetFabricTable().FabricCount();

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute_t *attr = attribute::get(light_endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    if (attr != nullptr && attribute::get_val(attr, &val) == ESP_OK) {
        on = val.val.b;
    }
    attr = attribute::get(light_endpoint_id, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);
    if (attr != nullptr && attribute::get_val(attr, &val) == ESP_OK) {
        level = val.val.u8;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    snprintf(line, sizeof(line), "Fabrics: %u", fabrics);
    oled_text(3, 0, line);

    snprintf(line, sizeof(line), "Light: %s Lvl:%u", on ? "ON " : "OFF", level);
    oled_text(4, 0, line);

    snprintf(line, sizeof(line), "Lenh nhan: %lu", (unsigned long)app_get_command_count());
    oled_text(6, 0, line);

    oled_text(7, 0, "Nut 3: doi trang");
}

/* Page 1 - one line per fabric */
static void app_oled_draw_fabrics()
{
    char line[OLED_COLS + 1];

    oled_text(0, 0, "CAC FABRIC");

    /* Copy the table out under the lock, format and draw outside it. The lock also
     * blocks the Matter task, and holding it for a whole redraw added jitter. */
    struct {
        uint8_t index;
        uint64_t node;
        uint16_t vendor;
    } rows[OLED_LINES - 2];

    uint8_t n_rows = 0;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    uint8_t count = chip::Server::GetInstance().GetFabricTable().FabricCount();
    for (const auto &info : chip::Server::GetInstance().GetFabricTable()) {
        if (n_rows >= sizeof(rows) / sizeof(rows[0])) {
            break; /* more fabrics than rows, show what fits */
        }
        rows[n_rows].index = info.GetFabricIndex();
        rows[n_rows].node = info.GetNodeId();
        rows[n_rows].vendor = info.GetVendorId();
        n_rows++;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    for (uint8_t i = 0; i < n_rows; i++) {
        /* Node IDs are 64-bit: print the low 32 bits and mark a truncated value */
        if (rows[i].node > 0xFFFFFFFFULL) {
            snprintf(line, sizeof(line), "#%u N:..%08lx", (unsigned)rows[i].index,
                     (unsigned long)(rows[i].node & 0xFFFFFFFFULL));
        } else {
            snprintf(line, sizeof(line), "#%u N:%lu V:%u", (unsigned)rows[i].index,
                     (unsigned long)rows[i].node, (unsigned)rows[i].vendor);
        }
        oled_text(i + 1, 0, line);
    }

    snprintf(line, sizeof(line), "Tong: %u fabric", count);
    oled_text(OLED_LINES - 1, 0, line);
}

/* Page 2 - counters */
static void app_oled_draw_counters()
{
    char line[OLED_COLS + 1];

    oled_text(0, 0, "BO DEM");

    snprintf(line, sizeof(line), "Lenh Matter: %lu", (unsigned long)app_get_command_count());
    oled_text(2, 0, line);

    snprintf(line, sizeof(line), "Nut 1 (toggle): %lu", (unsigned long)app_get_button_count(0));
    oled_text(3, 0, line);

    snprintf(line, sizeof(line), "Nut 2 (switch): %lu", (unsigned long)app_get_button_count(1));
    oled_text(4, 0, line);

    snprintf(line, sizeof(line), "Nut 3 (trang): %lu", (unsigned long)app_get_button_count(2));
    oled_text(5, 0, line);

    int64_t up = esp_timer_get_time() / 1000000;
    snprintf(line, sizeof(line), "Uptime: %llus", (unsigned long long)up);
    oled_text(7, 0, line);
}

void app_oled_update()
{
    if (!oled_is_ready()) {
        return;
    }

    oled_clear();
    switch (app_get_oled_page()) {
    case 0:
        app_oled_draw_status();
        break;
    case 1:
        app_oled_draw_fabrics();
        break;
    default:
        app_oled_draw_counters();
        break;
    }

    esp_err_t err = oled_flush();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "oled_flush failed: %s", esp_err_to_name(err));
    }
}
