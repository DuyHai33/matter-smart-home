# 03 — Build và nạp firmware lên ESP32-C6

## Bước 0 — Nạp môi trường

```bash
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh
```

## Bước 1 — Kiểm tra board nhận cổng

Cắm ESP32-C6 vào USB (cáp phải có dây data), rồi:

```bash
ls -l /dev/ttyACM*
dmesg | tail -20
```

Phải thấy `/dev/ttyACM0`. Nếu không thấy:
- Đổi cáp USB-C khác
- `sudo apt remove brltty` rồi rút cắm lại
- Kiểm tra nhóm: `groups | grep dialout`

## Bước 2 — Copy example ra thư mục project

Không sửa trực tiếp trong `~/esp/esp-matter` — sẽ mất khi update SDK.

```bash
cp -r $ESP_MATTER_PATH/examples/light  ~/matter-capstone/firmware/light
cd ~/matter-capstone/firmware/light
```

## Bước 3 — Build thử bản gốc trước khi sửa gì

```bash
idf.py set-target esp32c6
idf.py build
```

Lần đầu mất 10–25 phút. **Chỉ sửa code sau khi bản gốc build + chạy được** — để biết chắc lỗi sau này là do mình, không phải do môi trường.

## Bước 4 — Xoá flash rồi nạp

Lần đầu tiên bắt buộc erase, nếu không NVS cũ sẽ làm commissioning fail:

```bash
idf.py -p /dev/ttyACM0 erase-flash
idf.py -p /dev/ttyACM0 flash monitor
```

Thoát monitor: `Ctrl + ]`

## Bước 5 — Lấy pairing code

Trong log monitor, tìm dòng có:
- `Manual pairing code: [34970112332]`
- `SetupQRCode: [MT:...]`

Lưu lại ngay. Mặc định của example:
- Setup passcode: `20202021`
- Discriminator: `3840`
- Vendor ID: `0xFFF1` (test VID) — chỉ dùng để test, không được dùng thương mại

## Bước 6 — Lưu log build cho báo cáo

```bash
idf.py -p /dev/ttyACM0 monitor | tee monitor-$(date +%F-%H%M).log
```

Xem kích thước firmware (đưa vào Chương IV):
```bash
idf.py size
idf.py size-components | head -30
```

## Bước 7 — Cấu hình Wi-Fi

Với Matter, **không hardcode SSID/password vào code**. Thiết bị nhận thông tin Wi-Fi
qua quá trình commissioning (cluster Network Commissioning). Đây là điểm khác biệt
so với ESP32 làm MQTT thông thường — nhớ nhấn mạnh trong báo cáo.

## Checkpoint

- [ ] `idf.py build` thành công, không warning nghiêm trọng
- [ ] Flash xong, monitor in ra log CHIP khởi động
- [ ] Đã lưu manual pairing code
- [ ] Đã lưu output `idf.py size` vào `logs/`

## Nếu board chỉ có 4MB flash

Xem `99-troubleshooting.md` mục "Partition không vừa 4MB".
