# 04 — Commissioning vào Home Assistant (Fabric #1)

Điều kiện tiên quyết: đã xong `01` và `03`.

## 🔴 BẮT BUỘC làm trước: bật BLE cho add-on Matter Server

Kiểm tra ngày 16/08/2026 cho thấy log add-on ghi:

```
INFO  Controller~ndHandler  BLE is disabled
WARN  Commission~ontroller  BLE is not enabled on this platform
```

ESP32-C6 khi chưa có credentials Wi-Fi **chỉ quảng bá qua BLE**. BLE tắt ⇒ Cách 2 dưới đây
chắc chắn fail. Phương án đã chọn cho đồ án: **dùng BLE onboard của Raspberry Pi 4**.

**✅ ĐÃ LÀM XONG 16/08/2026 — dùng chế độ `ble_proxy`.**

1. Settings → Add-ons → **Matter Server** → tab **Configuration**
2. Bấm **⋮ → Show unused optional configuration options**
3. Bật **`ble_proxy`**
4. **Save** → tab Info → **Restart** add-on

> ⚠️ **Đừng đặt `bluetooth_adapter_id`.** Hai tuỳ chọn loại trừ nhau, add-on sẽ log:
> `WARNING: Ignoring 'bluetooth_adapter_id' option: 'ble_proxy' is enabled (mutually exclusive)`.
> Chế độ proxy để **HA Core** giữ Bluetooth rồi chuyển tiếp qua WebSocket `/ble`, nên
> **không cần xoá integration Bluetooth** của HA — không có tranh chấp adapter `hci0`.

Kiểm chứng trong tab Log, phải thấy:
```
INFO  MatterServer          BLE proxy mode enabled
INFO  Controller~ndHandler  BLE is enabled (proxy mode)
INFO  BleProxyHandler       BLE proxy WebSocket endpoint registered on /ble
INFO  BleProxyConnection    [ble0] BLE proxy handshake complete (version 1)
```
Còn thấy `BLE is disabled` nghĩa là chưa Save/Restart.

## Nạp trước Wi-Fi credentials cho Matter Server

Log add-on ghi `Set config key wifiSsid to undefined` — Matter Server chưa có Wi-Fi để cấp
cho thiết bị lúc commissioning. Nạp qua panel riêng của add-on (**không** phải qua service
của HA — `/api/services` không có `matter.set_wifi_credentials`):

1. Sidebar HA → **Matter Server** → biểu tượng ⚙ (Settings) góc trên phải
2. Mục **WiFi** → **+ Add**
3. Điền:

| Ô | Giá trị | Ghi chú |
|---|---|---|
| Identifier | `fpt-iot` | chỉ là nhãn phân biệt, **không** phải tên mạng. Cấm đặt `default` / `delete` |
| SSID | `FPT Telecom-3D02 - IoT` | gõ chính xác, có khoảng trắng hai bên dấu `-` |
| Password | *(mật khẩu Wi-Fi đã đặt)* | bấm 👁 để soi lại trước khi Save |

4. **Save**
5. Kiểm dòng `Not configured [DEFAULT]` phía trên — nếu vẫn `Not configured`, bấm **+ Set**
   ngay dòng đó và nhập lại cùng SSID/password. **DEFAULT mới là bộ được dùng khi commissioning**;
   bộ có Identifier là dành cho trường hợp nhiều mạng.

**Kiểm chứng:** mở tab Log của add-on, dòng `ConfigStorage  Set config key wifiSsid to ...`
phải hiện giá trị thật thay vì `undefined`.

> SSID đã đối chiếu với sóng thật bằng `netsh wlan show networks mode=bssid` trên Windows:
> `FPT Telecom-3D02 - IoT`, Band 2.4 GHz, Channel 7, Signal 99%.

## Kiểm tra trước khi bấm

- [ ] Pi (HAOS) và ESP32-C6 sẽ vào **cùng một router**
- [ ] Router bật băng tần **2.4GHz** (C6 không bắt được 5GHz) — SSID: `FPT Telecom-3D02 - IoT`
- [ ] SSID đó **không bị cách ly** khỏi LAN của Pi
- [ ] IPv6 bật trên router ✔ (đã xác nhận)
- [ ] BLE của add-on đã bật (mục ở trên)
- [ ] Timezone HA đã đổi sang `Asia/Ho_Chi_Minh` (nếu không, mốc thời gian đo sẽ lệch 7h)
- [ ] ESP32-C6 đang cắm USB và đã flash firmware, chưa từng commission (hoặc đã `erase-flash`)

## Cách 1 — Qua app Home Assistant trên điện thoại (dễ nhất)

App HA (Android/iOS) có sẵn bộ commissioning của hệ điều hành.

1. Điện thoại kết nối **cùng Wi-Fi** với Pi
2. Mở app HA → **Settings → Devices & Services → Add Integration → Matter**
3. Chọn "Add Matter device"
4. Quét QR code (in ra từ URL trong log) hoặc chọn nhập mã thủ công → gõ manual pairing code
5. Chọn mạng Wi-Fi 2.4GHz và nhập password khi được hỏi
6. Chờ 1–3 phút

## Cách 2 — Qua giao diện web (không cần điện thoại)

Bản HA mới cho phép nhập pairing code trực tiếp trên web, nhưng thiết bị Wi-Fi
**chưa vào mạng** thì cần bộ commissioner có Bluetooth. Nếu Pi có BLE (Pi 4 có sẵn),
HA có thể tự làm qua BLE.

1. Web HA → **Settings → Devices & Services → Matter → Add device**
2. Nhập manual pairing code
3. Chọn Wi-Fi

Nếu báo lỗi không tìm thấy thiết bị qua BLE → dùng Cách 1 hoặc Cách 3.

## Cách 3 — Commission bằng chip-tool trước, rồi share sang HA

Dùng khi hai cách trên fail. chip-tool đưa thiết bị lên Wi-Fi qua BLE:

```bash
cd ~/esp/esp-matter/connectedhomeip/connectedhomeip/out/host

# Node ID 1, đưa thiết bị vào Wi-Fi luôn
./chip-tool pairing ble-wifi 1 "<SSID>" "<PASSWORD>" 20202021 3840
```

Sau đó mở commissioning window để HA vào (xem `05-multi-fabric-chip-tool.md`).

## Sau khi commission thành công

Trong HA sẽ xuất hiện một device Matter với các entity:
- `light.<ten>` — bật/tắt, độ sáng, màu

Việc cần làm ngay:
- [ ] Bật/tắt từ HA, xác nhận LED phản hồi
- [ ] Đo độ trễ từ lúc bấm đến lúc LED đổi → ghi vào `data/bang-4-2-do-tre.csv`
- [ ] Chụp màn hình device page trong HA
- [ ] Vào device → xem Node ID, Fabric Index → ghi lại

## Dữ liệu cần thu để đưa vào Chương IV

| Thông tin | Lấy ở đâu |
|---|---|
| Thời gian commissioning (giây) | Bấm đồng hồ từ lúc bắt đầu đến khi HA báo xong |
| Số lần thử / số lần thành công | Lặp lại ít nhất 10 lần với `erase-flash` giữa các lần |
| Node ID, Fabric Index | Device info trong HA |
| Log commissioning | Add-on Matter Server → tab Log → copy vào `logs/` |
| RSSI Wi-Fi của C6 | Log monitor của ESP32 |

## Lỗi hay gặp

Xem `99-troubleshooting.md`. Ba lỗi phổ biến nhất:
1. Thiết bị không hiện lên → mDNS/IPv6 bị chặn
2. Commissioning fail ở bước cuối (~90%) → sai SSID/password, hoặc đang là mạng 5GHz
3. `Device already commissioned` → cần `erase-flash` lại
