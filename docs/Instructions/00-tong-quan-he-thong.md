# 00 — Tổng quan hệ thống

## Mục tiêu đồ án

Thi công một thiết bị Matter thật (ESP32-C6), ghép vào Home Assistant chạy trên Raspberry Pi 4,
và **chứng minh đặc tính multi-admin của Matter**: cùng một thiết bị vật lý được điều khiển đồng thời
bởi nhiều controller thuộc các fabric khác nhau, không cần cloud, không cần app riêng của hãng.

Đây là điểm khác biệt so với các đồ án IoT thông thường dùng MQTT/HTTP tự chế.

## Sơ đồ khối

```
                        ┌──────────────────────────┐
                        │      Router Wi-Fi        │
                        │  (IPv6 + mDNS bật)       │
                        └───┬──────────────┬───────┘
                    Ethernet│              │Wi-Fi 2.4GHz
                    ┌───────▼──────┐   ┌───▼────────────────┐
                    │ Raspberry Pi4│   │  ESP32-C6 SuperMini│
                    │  HAOS        │   │  Matter End Device │
                    │  + Matter    │   │  EP1: Color Light  │
                    │    Server    │   │  EP2: Generic Sw   │
                    │  = Fabric #1 │   │  + OLED SSD1306    │
                    └──────────────┘   └────────────────────┘
                    ┌──────────────┐            ▲
                    │ Ubuntu 26.04 │            │
                    │ chip-tool x2 ├────────────┘
                    │ = Fabric #2,3│
                    └──────────────┘
```

## Vai trò từng khối

| Khối | Vai trò |
|---|---|
| ESP32-C6 | Matter End Device. Chạy stack Matter (CHIP) trên ESP-IDF, expose 2 endpoint |
| Raspberry Pi 4 + HAOS | Matter Controller/Admin thứ nhất, chạy `python-matter-server` dạng add-on |
| Ubuntu PC | (a) máy build firmware; (b) chạy chip-tool đóng vai controller thứ 2, 3 |
| Router | Cung cấp L2 chung, IPv6 link-local, forward mDNS. **Đây là điểm hay hỏng nhất** |

## Vì sao chọn Matter over Wi-Fi thay vì Thread

| | Wi-Fi | Thread |
|---|---|---|
| Phần cứng thêm | Không | Cần Thread Border Router (HomePod / Nest / RPi + dongle 802.15.4) |
| Băng thông | Cao | Thấp |
| Tiêu thụ điện | Cao hơn | Thấp hơn |
| Phù hợp đồ án | ✔ Làm được ngay với đồ đang có | ✘ Phải mua thêm, rủi ro trễ tiến độ |

Ghi chú cho báo cáo: ESP32-C6 **có** radio 802.15.4 nên về lý thuyết chạy được Thread.
Nêu điều này trong phần "Hướng phát triển" — thể hiện là hiểu rõ chứ không phải bị giới hạn kiến thức.

## Mô hình dữ liệu Matter dự kiến

| Endpoint | Device Type | Cluster chính | Ánh xạ phần cứng |
|---|---|---|---|
| 0 | Root Node | Basic Information, Network Commissioning, Operational Credentials | — |
| 1 | Extended Color Light | On/Off, Level Control, Color Control | LED RGB |
| 2 | Generic Switch | Switch (InitialPress / ShortRelease event) | Nút nhấn 2 |

Nút nhấn 1 không tạo endpoint riêng — nó tác động trực tiếp lên attribute OnOff của EP1
(mô phỏng công tắc vật lý), để chứng minh trạng thái được đồng bộ ngược lên tất cả fabric.

## Kịch bản demo cuối cùng

1. Bật đèn từ Home Assistant → LED sáng → chip-tool #2 đọc attribute thấy OnOff = 1
2. Đổi màu từ chip-tool #2 → LED đổi màu → giao diện HA cập nhật màu theo
3. Nhấn nút vật lý trên board → LED tắt → cả HA lẫn chip-tool đều thấy trạng thái mới
4. OLED hiển thị số fabric đang kết nối, tăng dần sau mỗi lần commissioning
5. Rút dây Internet của router (giữ LAN nội bộ) → mọi thứ vẫn chạy → chứng minh Matter là local control

Kịch bản 5 là điểm ăn tiền khi bảo vệ — nhớ quay video lại.
