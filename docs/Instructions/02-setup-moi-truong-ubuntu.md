# 02 — Môi trường ESP-IDF + esp-matter trên Ubuntu (WSL2)

**Trạng thái: ĐÃ HOÀN THÀNH.** File này để tham khảo khi cần cài lại từ đầu.

## 🔴 Ubuntu này là WSL2, không phải máy Linux riêng

Phát hiện 16/08/2026. Bằng chứng từ chính máy:

```
inet 10.255.255.254/32 scope global lo              <- dac trung WSL2
inet 192.168.215.110/20 brd 192.168.223.255 eth0    <- subnet NAT cua WSL2
inet6 fe80::215:5dff:fe98:fd2                       <- MAC 00:15:5D = OUI Hyper-V
ls: cannot access '/dev/ttyACM0'                    <- WSL2 khong thay USB
```

Phía Windows: WSL `2.7.11.0`, kernel `6.18.33.2-2`, Windows 11 build `26200`, distro `Ubuntu` (VERSION 2).

**Hai hệ quả bắt buộc phải xử lý:**

### (1) Mạng NAT → Matter không chạy

WSL2 mặc định nằm sau NAT, subnet `192.168.215.0/20`, **khác lớp 2** với Pi (`192.168.1.196`).
Matter dựa vào **mDNS (multicast 5353)** và **IPv6 link-local** — cả hai đều không đi qua NAT.
Hậu quả: `chip-tool` sẽ treo vĩnh viễn ở `Discovering`, Giai đoạn 4 (multi-fabric) bất khả thi.

**Khắc phục — bật mirrored networking.** Tạo `C:\Users\<user>\.wslconfig`:

```ini
[wsl2]
networkingMode=mirrored
dnsTunneling=true
autoProxy=true
firewall=true

[experimental]
hostAddressLoopback=true
```

Rồi từ PowerShell:
```powershell
wsl --shutdown
```
Mở lại terminal Ubuntu, kiểm chứng:
```bash
ip -4 addr show | grep inet        # phai thay 192.168.1.x (giong may Windows)
ip -6 addr show | grep inet6       # phai co dia chi 2405:4802:... (IPv6 global)
ping -c 3 192.168.1.196            # ping duoc Pi
```
Chưa đạt cả 3 → **đừng đi tiếp sang Giai đoạn 4**, xem `99-troubleshooting.md` mục J.

### (2) Không có `/dev/ttyACM0` → không flash được

WSL2 không nhìn thấy USB. Phải chuyển cổng USB từ Windows sang WSL bằng **usbipd-win**
(đã cài sẵn trên máy này, bản `5.3.0`).

```powershell
# PowerShell voi quyen Administrator, sau khi cam ESP32-C6
usbipd list                        # tim dong ESP32-C6 / USB JTAG-serial, ghi lai BUSID
usbipd bind   --busid <BUSID>      # chi can lam 1 lan cho moi thiet bi
usbipd attach --wsl --busid <BUSID>
```
Rồi trong Ubuntu:
```bash
ls -l /dev/ttyACM0
sudo usermod -aG dialout $USER     # neu bao Permission denied, xong dang xuat/vao lai
```
Rút board ra cắm lại là phải `usbipd attach` lại.

## Cấu hình đã dùng

| Thành phần | Phiên bản |
|---|---|
| Ubuntu | 26.04 LTS (resolute) — **chạy trên WSL2**, không phải bare-metal |
| Python | 3.12.13 |
| ESP-IDF | v5.5.4 tại `~/esp/esp-idf` |
| esp-matter | `~/esp/esp-matter` |
| Target | esp32c6 |

⚠️ **Không nâng version.** Nếu build lại, giữ đúng v5.5.4.

## Các lệnh đã chạy (để tái lập)

```bash
# 1. Gói phụ thuộc
sudo apt update
sudo apt install -y git wget flex bison gperf python3 python3-pip python3-venv \
  cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
  gcc g++ pkg-config curl libdbus-1-dev libglib2.0-dev libavahi-client-dev \
  unzip libgirepository1.0-dev libcairo2-dev libreadline-dev libevent-dev default-jre

# 2. Cấu hình git cho mạng chậm (đã từng bị timeout khi clone)
git config --global http.lowSpeedLimit 1000
git config --global http.lowSpeedTime 600
git config --global http.postBuffer 524288000

# 3. ESP-IDF
mkdir -p ~/esp && cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && git checkout v5.5.4
git submodule update --init --recursive
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"   # mirror, tải nhanh hơn ở VN
./install.sh esp32c6

# 4. esp-matter
cd ~/esp
source esp-idf/export.sh
git clone --depth 1 https://github.com/espressif/esp-matter.git
cd esp-matter
git submodule update --init --depth 1
cd connectedhomeip/connectedhomeip
./scripts/checkout_submodules.py --platform esp32 linux --shallow
cd ../..
./install.sh
```

## Quyền cổng serial

```bash
sudo usermod -a -G dialout $USER
sudo apt remove -y brltty        # brltty hay chiếm /dev/ttyACM0
# logout/login lại để nhóm dialout có hiệu lực
```

## avahi cho mDNS (cần cho chip-tool)

```bash
sudo apt install -y avahi-daemon avahi-utils
sudo systemctl enable --now avahi-daemon
```

## Mỗi lần mở terminal mới

```bash
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh
```

Nên gói hai dòng trên vào một file rồi `source` cho nhanh.

Alias cho tiện:
```bash
echo "alias getidf='source ~/esp/esp-idf/export.sh && source ~/esp/esp-matter/export.sh'" >> ~/.bashrc
```

## Kiểm tra chip-tool

```bash
~/esp/esp-matter/connectedhomeip/connectedhomeip/out/host/chip-tool --help
```
