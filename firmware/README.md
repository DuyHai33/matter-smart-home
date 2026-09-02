# firmware/ — Mã nguồn ESP32-C6

`firmware/light/` là **bản sao mã nguồn đang chạy thật trên board**. Đây chính là firmware đã
commission thành công vào 5 fabric.

> ⚠️ **Đây là bản sao để đọc và để trích vào phụ lục báo cáo, không phải nơi build.**
> Build diễn ra ở `~/light-capstone` trong WSL (ổ Linux, nhanh hơn nhiều lần so với build trên
> `/mnt/c`). Sửa code ở đâu cũng được, nhưng phải đồng bộ hai bên trước khi build.

## Nguồn gốc

Bắt đầu từ example `light` của esp-matter:

```bash
cp -r $ESP_MATTER_PATH/examples/light ~/light-capstone
```

## Cấu trúc

```
firmware/light/
├── CMakeLists.txt
├── partitions.csv
├── sdkconfig.defaults
├── sdkconfig.capstone-redacted   <- cấu hình thật, ĐÃ BÔI mật khẩu Wi-Fi
├── dependencies.lock
└── main/
    ├── CMakeLists.txt
    ├── Kconfig.projbuild
    ├── app_main.cpp              <- điểm vào, dựng Matter node, EP1 + EP2
    ├── app_driver.cpp            <- driver đèn, LED chỉ báo, 3 nút nhấn
    ├── app_oled.cpp              <- 3 trang nội dung của màn OLED
    ├── oled_ssd1306.c / .h       <- driver SSD1306 tự viết
    ├── app_priv.h
    ├── idf_component.yml
    └── certification_declaration/
```

Không copy `build/` và `managed_components/` — dung lượng lớn, sinh lại được bằng `idf.py build`.

## Đã thêm so với example gốc

| Phần | File | Ghi chú |
|---|---|---|
| Endpoint 2 — Generic Switch | `app_main.cpp`, `app_driver.cpp` | Phát event `InitialPress` / `ShortRelease`. Cluster phải khai feature `MomentarySwitch` + `MomentarySwitchRelease` ngay trong config, và phải đăng ký event thì lệnh gửi mới có chỗ đi |
| 3 LED chỉ báo | `app_driver.cpp` | GPIO output thường: mạng / on-off / nhấp nháy khi có lệnh |
| 3 nút nhấn | `app_driver.cpp` | toggle EP1, generic switch EP2, đổi trang OLED |
| Driver SSD1306 | `oled_ssd1306.c` | Tự viết thay vì lấy từ component registry: phân vùng app chỉ còn ~7% trống |
| 3 trang OLED | `app_oled.cpp` | Trang 1 liệt kê từng fabric — nhìn thấy multi-admin ngay trên thiết bị |

Gọi vào CHIP stack từ callback của nút phải qua `PlatformMgr().ScheduleWork()`. Gọi thẳng thì
stack gọi `chipDie()` vì đang chạy sai task.

## Bảo mật

`sdkconfig` thật ở WSL chứa **mật khẩu Wi-Fi dạng rõ** tại `CONFIG_DEFAULT_WIFI_PASSWORD`. Bản
trong repo (`sdkconfig.capstone-redacted`) đã thay bằng `<DA-BOI-DEN>`. **Không bao giờ copy
nguyên bản `sdkconfig` vào repo hoặc vào báo cáo.**

## Cấu hình then chốt đã sửa so với example gốc

| Tham số | Giá trị | Lý do |
|---|---|---|
| `CONFIG_DEFAULT_WIFI_SSID` | *(SSID 2.4GHz riêng cho IoT)* | Cấp Wi-Fi thẳng vào firmware vì commissioning qua BLE không chạy |
| `CONFIG_DEFAULT_WIFI_PASSWORD` | *(đã bôi)* | Board tự lên mạng, controller tìm qua mDNS |

Cơ chế: `connectedhomeip/src/platform/ESP32/ConnectivityManagerImpl_WiFi.cpp:353-379` — nếu
`!IsWiFiStationProvisioned()` và SSID khác rỗng thì tự `esp_wifi_set_config`.
**Bắt buộc `erase-flash` trước**, vì NVS còn provision cũ thì nhánh này bị bỏ qua.

## Hạn chế đã biết của firmware này

- **Không hỗ trợ Hue/Saturation** — `ColorCapabilities = 24` (chỉ XY + ColorTemperature).
  Điều khiển màu phải đi đường toạ độ XY.
- **`move-to-level` xoá màu**: `led_driver_set_brightness()` luôn đi qua
  `hsv_to_rgb(current_HS, ...)` với `current_HS = {0,0}` (saturation 0 = thang xám), và biến này
  không bao giờ được cập nhật vì `ColorCapabilities = 24`. Kết quả là mọi lệnh level đều biến
  LED thành trắng. **Luôn đặt level trước, màu sau.**
- **Không có attribute `CurrentHeapFree`** → không đo được free heap qua Matter (bảng 4-6).

## Nạp lại firmware ảnh hưởng tới 5 fabric

| Việc | Hậu quả |
|---|---|
| `write-flash` chỉ app (`0x20000 light.bin`) | không chạm NVS → nhiều khả năng giữ được fabric |
| `erase-flash` / nạp cả 4 binary | **mất sạch 5 fabric**, phải commission lại từ đầu |
| Nạp firmware test khác (i2c_tools, blink) | **đè phân vùng app, mất firmware Matter** |
