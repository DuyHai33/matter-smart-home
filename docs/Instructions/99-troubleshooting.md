# 99 — Xử lý sự cố

## A. Mạng chặn IPv6 / mDNS

**Triệu chứng**: HA không tìm thấy thiết bị; chip-tool treo ở `Discovering`;
`ping homeassistant.local` không phân giải.

**Kiểm tra**:
```bash
ip -6 addr show | grep -i "inet6 fe80"     # phải có link-local
systemctl status avahi-daemon
avahi-browse -a -t | head -20
avahi-browse -rt _matter._tcp              # thiết bị đã commission
avahi-browse -rt _matterc._udp             # thiết bị đang chờ commission
```

**Khắc phục**:
1. Không dùng Wi-Fi trường / ký túc xá. Dùng router nhà hoặc hotspot điện thoại có bật IPv6.
2. Trên router: bật IPv6, tắt "AP isolation" / "Client isolation", tắt "IGMP/MLD snooping" nếu chặn multicast.
3. Kiểm tra firewall Ubuntu: `sudo ufw status`. Nếu active thì `sudo ufw allow 5353/udp`.
4. Pi cắm dây LAN vào **chính router đó**, không qua switch/repeater khác.

## B. Không thấy /dev/ttyACM0

```bash
dmesg | tail -20          # xem kernel có nhận USB không
sudo apt remove -y brltty # brltty chiếm cổng ACM
groups | grep dialout     # phải có dialout
```
Nếu `dmesg` không thấy gì khi cắm → **cáp USB không có dây data**, đổi cáp.

## C. Partition không vừa 4MB flash

**Triệu chứng**: build báo `Error: app partition is too small` hoặc flash fail.

> ✅ Đã xác nhận 16/08/2026: board dùng chip `ESP32C6FN4` → **flash 4MB**. Mục này sẽ cần dùng.

> ⚠️ **Đính chính**: bản trước của file này nói esp-matter có sẵn `partitions_4mb.csv` trong
> `examples/light/`. **Sai.** `ls` thực tế chỉ có: `partitions.csv`, `partitions.s31.csv`,
> `partitions_wifi_thread.csv`. Không có file 4MB riêng.
> Nhưng có `sdkconfig.defaults.esp32c6` — dùng cái này trước, rất có thể đã vừa 4MB.

**Cách 1 — dùng sdkconfig.defaults của C6 rồi build thử**:
```bash
cd $ESP_MATTER_PATH/examples/light
cat partitions.csv                     # xem partition mac dinh chiem bao nhieu
idf.py set-target esp32c6
idf.py build
```
Build lọt thì khỏi làm gì thêm. Báo `app partition is too small` mới sang Cách 2/3.

**Cách 1b — chỉnh tay trong menuconfig**:
```bash
idf.py menuconfig
```
→ `Serial flasher config` → `Flash size` = **4 MB**
→ `Partition Table` → `Custom partition table CSV` → trỏ tới file CSV tự sửa
   (copy `partitions.csv` ra rồi thu nhỏ/bỏ bớt phân vùng)

**Cách 2 — giảm dung lượng firmware**:
```
Component config → Log output → Default log verbosity = Warning
Component config → CHIP Device Layer → tắt các tính năng không dùng
Compiler options → Optimization Level = Optimize for size (-Os)
```

**Cách 3 — tắt OTA** (project không cần OTA):
Trong partition CSV, bỏ phân vùng `ota_1`, gộp dung lượng cho `factory`.

## D. Commissioning fail

| Lỗi trong log | Nguyên nhân | Cách sửa |
|---|---|---|
| `Device already commissioned` | NVS còn fabric cũ | `idf.py -p /dev/ttyACM0 erase-flash` rồi flash lại |
| Fail ở ~90%, thiết bị reboot | Sai SSID/password, hoặc là mạng 5GHz | Tách SSID 2.4GHz riêng, nhập lại |
| `Failed to discover device` | mDNS/IPv6 | xem mục A |
| `CASE session establishment failed` | Đồng hồ hệ thống lệch | `sudo timedatectl set-ntp true` trên Pi và Ubuntu |
| Timeout khi quét BLE | Pi chưa bật Bluetooth | Settings → System → Hardware, hoặc commission qua app điện thoại |

## E. chip-tool báo lỗi

```
CHIP Error 0x00000032: Timeout
```
→ Thiết bị không phản hồi. Kiểm tra thiết bị còn online: `avahi-browse -rt _matter._tcp`

```
Failed to open storage
```
→ Thư mục `--storage-directory` chưa tồn tại. `mkdir -p` trước.

**Mất fabric sau khi reboot máy**: chip-tool lưu credential trong `--storage-directory`.
Nếu không truyền tham số này, nó dùng `/tmp/chip_*` — **mất khi reboot**.
Luôn truyền `--storage-directory`.

## F. Add-on Matter Server không start

- Xem tab Log của add-on
- Kiểm tra HAOS còn dung lượng: **Settings → System → Storage**
- Restart add-on, nếu vẫn lỗi thì uninstall → reinstall
- Đảm bảo Pi dùng nguồn 5V/3A thật; nguồn yếu gây lỗi ngẫu nhiên khó đoán

## G. Build ESP-IDF lỗi sau khi mở terminal mới

Quên source environment. Chạy:
```bash
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh
```
**Đúng thứ tự này.** Source esp-matter trước sẽ lỗi.

## H. Thẻ SD hỏng / HAOS không boot

Thẻ SD kém chất lượng là nguyên nhân số 1 gây lỗi HAOS ngẫu nhiên.
Dùng thẻ A1/A2, UHS-I Class 10 của hãng uy tín. Nếu Pi boot loop → flash lại thẻ mới.

## J. WSL2 — không thấy USB, không discover được thiết bị Matter

Môi trường Ubuntu của đồ án này chạy trên **WSL2**, không phải Linux bare-metal.
Xem `02-setup-moi-truong-ubuntu.md` để biết chi tiết.

**Triệu chứng 1 — `ls: cannot access '/dev/ttyACM0'`** dù board đã cắm và Windows nhận.

### ❌ usbipd KHÔNG chạy được với WSL mirrored networking (đã thử, bỏ)

Ngày 16/08/2026 đã thử usbipd-win 5.3.0 và **thất bại 3 lần**, luôn cùng một triệu chứng:
thiết bị enumerate thành công rồi bị ngắt sau 50–700 ms.

```
[677.830161] cdc_acm 1-1:1.0: ttyACM0: USB ACM device   <- hien ra
[677.855734] vhci_hcd: connection closed                 <- 25ms sau bi dut
[677.856017] usb 1-1: USB disconnect, device number 10
```

Đã loại trừ các nguyên nhân sau bằng đo đạc, **đừng thử lại**:
- Firewall Hyper-V chặn cổng 3240 → **sai**. Test từ WSL: `bash -c '</dev/tcp/127.0.0.1/3240'` → mở.
  (3 rule `New-NetFirewallHyperVRule` đã thêm là thừa cho usbipd, nhưng **giữ lại rule
  UDP 5353 + 5540** vì Giai đoạn 4 cần.)
- Chưa `bind` → **sai**, `usbipd list` báo STATE = `Shared`.
- Tiến trình attach thoát sớm → **sai**, `--auto-attach` cũng đứt y hệt.

### ✅ Cách dùng thật: build trong WSL, nạp từ Windows

USB không cần vào WSL. Chỉ khâu nạp cần cổng COM, mà Windows có sẵn.

**Chuẩn bị 1 lần** (PowerShell thường, không cần admin):
```powershell
py -m pip install --user esptool
py -m esptool version                       # kiem tra
py -m esptool --port COM6 flash-id          # kiem tra board
```

**Nạp firmware** — sau khi `idf.py build` xong trong WSL:
```powershell
# Duong dan WSL nhin tu Windows: \\wsl.localhost\Ubuntu\home\<user>\<project>\build
Push-Location "\\wsl.localhost\Ubuntu\home\duyhai\light-capstone\build"
py -m esptool --chip esp32c6 --port COM6 erase-flash
py -m esptool --chip esp32c6 --port COM6 --baud 460800 write-flash "@flash_args"
Pop-Location
```

**Xem log serial** (thay cho `idf.py monitor`):
```powershell
py -m serial.tools.miniterm COM6 115200
```
Thoát bằng `Ctrl+]`.

> Nếu `write-flash "@flash_args"` lỗi vì đường dẫn UNC, copy thư mục `build` sang ổ C rồi nạp:
> trong WSL chạy `cp -r ~/light-capstone/build /mnt/c/Users/nguye/Downloads/build-c6`

**Triệu chứng 2 — `chip-tool` treo ở `Discovering`, `avahi-browse` không thấy gì**:

Kiểm tra trước:
```bash
ip -4 addr show | grep inet
```
- Ra `192.168.215.x` hoặc `172.x` → đang ở chế độ **NAT**, mDNS/IPv6 không qua được. **Phải bật mirrored networking** (xem `02`).
- Ra `192.168.1.x` giống máy Windows → mạng đã đúng, lỗi nằm chỗ khác, xem mục A.

**Sau khi bật mirrored, firewall Windows có thể chặn mDNS.** Nếu vẫn không thấy:
```powershell
# PowerShell Administrator - cho phep mDNS di vao
New-NetFirewallRule -DisplayName "mDNS-in-UDP5353" -Direction Inbound -Protocol UDP -LocalPort 5353 -Action Allow
```
Và trong WSL:
```bash
sudo apt install -y avahi-daemon avahi-utils
sudo service avahi-daemon start
avahi-browse -rt _matterc._udp
```
> WSL2 không chạy systemd mặc định → dùng `sudo service ...` chứ không phải `systemctl`.

## I. Lọc log dài trước khi đọc

Log serial của Matter rất dài. Lọc trước cho đỡ mất thời gian:
```bash
# Chỉ lấy lỗi
grep -iE "error|fail|assert|abort" logs/monitor.log | head -50

# 60 dòng cuối
tail -60 logs/monitor.log

# Lọc log CHIP
grep "CHIP:" logs/monitor.log | tail -40
```
