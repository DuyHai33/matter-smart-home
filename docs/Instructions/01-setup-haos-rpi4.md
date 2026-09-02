# 01 — Cài Home Assistant OS lên Raspberry Pi 4

**Trạng thái**: đang làm. Khoảng 60–90 phút, phần lớn là chờ.

## Chuẩn bị

- Thẻ microSD 32GB + đầu đọc thẻ ✔
- Cáp micro-HDMI → HDMI ✔ (chỉ dùng khi cần debug)
- Dây LAN cắm Pi vào router (**khuyến nghị mạnh**, ổn định hơn Wi-Fi nhiều)
- Nguồn USB-C 5V/3A cho Pi 4

## Bước 1 — Flash HAOS lên thẻ SD

Trên máy Ubuntu:

```bash
sudo apt install -y rpi-imager
rpi-imager
```

Trong Raspberry Pi Imager:
1. **Choose Device** → Raspberry Pi 4
2. **Choose OS** → `Other specific-purpose OS` → `Home assistants and home automation` → `Home Assistant` → bản cho **RPi 4/400 (64-bit)**
3. **Choose Storage** → thẻ SD trong đầu đọc (kiểm tra kỹ, đừng chọn nhầm ổ cứng)
4. **Next** → khi hỏi customisation chọn **No** (HAOS không dùng được customisation của Imager)
5. Chờ ghi + verify, khoảng 5–15 phút

## Bước 2 — Boot Pi

1. Cắm thẻ SD vào Pi
2. Cắm dây LAN vào router
3. (Tuỳ chọn) cắm micro-HDMI vào màn hình để xem log boot
4. Cắm nguồn

**Lần boot đầu mất 10–20 phút** (Pi tự resize partition, tải core container). Đừng rút điện giữa chừng.

## Bước 3 — Truy cập giao diện

Từ máy Ubuntu, thử theo thứ tự:

```bash
# Cách 1 - mDNS
ping -c 3 homeassistant.local

# Cách 2 - nếu mDNS không chạy, quét mạng tìm IP của Pi
sudo apt install -y nmap
ip route | grep default          # xem subnet, ví dụ 192.168.1.0/24
sudo nmap -sn 192.168.1.0/24
```

Mở trình duyệt: `http://homeassistant.local:8123` hoặc `http://<IP-của-Pi>:8123`

> ⚠️ **Trên setup thực tế của đồ án này, HA lại nghe ở port 80** chứ không phải 8123 →
> dùng `http://<IP-cua-Pi>/`.
> Nếu `:8123` không vào được, quét port trước khi nghi ngờ mạng:
> ```powershell
> foreach ($p in 80,8123,4357,5580) { "$p : " + (Test-NetConnection 192.168.1.196 -Port $p -InformationLevel Quiet) }
> ```

Không vào được → cắm màn hình HDMI vào Pi, console HAOS sẽ in ra IP.
Trên console gõ `network info` để xem chi tiết.

## Bước 4 — Tạo tài khoản

Điền tên, username, password. Ghi lại chỗ riêng — tuyệt đối không commit vào repo.
Onboarding hỏi vị trí, đơn vị đo → TP.HCM, hệ mét.

## Bước 5 — Cài Matter Server add-on

1. **Settings → Add-ons → Add-on Store**
2. Tìm `Matter Server` (do Home Assistant phát hành)
3. **Install** → chờ vài phút
4. Bật **Start on boot** và **Watchdog**
5. **Start**
6. Vào tab **Log** của add-on, xác nhận không có ERROR

## Bước 6 — Thêm integration Matter

1. **Settings → Devices & Services → Add Integration**
2. Tìm `Matter` → chọn
3. Chọn dùng add-on chính thức → Submit
4. Integration hiện lên với 0 thiết bị — đúng, vì chưa commission gì

## Bước 7 — Kiểm tra điều kiện mạng cho Matter

```bash
ip -6 addr show | grep inet6      # phai co IPv6 link-local fe80::
avahi-resolve -n homeassistant.local
```

Cần đạt:
- Máy có địa chỉ IPv6 link-local (`fe80::...`)
- `avahi-daemon` đang chạy
- Phân giải được `homeassistant.local`
- Pi và Ubuntu **cùng subnet**

Thiếu IPv6 hoặc mDNS → xem `99-troubleshooting.md`, mục "Mạng chặn IPv6/mDNS".

## Checkpoint

- [x] Vào được giao diện HA (`http://192.168.1.196/`, port 80), đã tạo tài khoản
- [ ] Add-on Matter Server ở trạng thái Running, log sạch
- [ ] Integration Matter đã thêm
- [ ] Ghi lại IP của Pi, đặt luôn DHCP reservation trên router cho khỏi đổi
- [ ] Chụp màn hình HA dashboard → dùng làm hình minh hoạ Chương IV
