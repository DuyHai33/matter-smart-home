# 07 — Kế hoạch đo đạc cho Chương IV

> **Nguyên tắc: chỉ ghi số đo thật.** Bảng nào chưa đo thì để trống.
> Đã cam kết trong Lời cam đoan của báo cáo.

## Thiết bị và phần mềm dùng để đo

| Mục | Công cụ |
|---|---|
| Độ trễ điều khiển | Quay video 60fps bằng điện thoại (màn hình + LED trong cùng khung hình), đếm frame |
| Thời gian commissioning | Đồng hồ bấm giờ / timestamp trong log |
| RSSI Wi-Fi | `esp_wifi_sta_get_ap_info()` in ra monitor |
| Dung lượng firmware | `idf.py size`, `idf.py size-components` |
| RAM sử dụng | `esp_get_free_heap_size()` in định kỳ |
| Log mạng | Add-on Matter Server → tab Log |
| Số fabric | `chip-tool operationalcredentials read fabrics` |

## Thí nghiệm 1 — Độ tin cậy commissioning

**Cách làm**: lặp 10 lần, mỗi lần `idf.py erase-flash` rồi commission lại vào HA.
Ghi thời gian từ lúc bấm "Add device" tới lúc HA hiện entity.

→ `data/bang-4-1-commissioning.csv`

## Thí nghiệm 2 — Độ trễ điều khiển

**Cách làm**: đặt điện thoại quay 60fps, trong khung hình có cả màn hình HA và LED.
Bấm bật/tắt, đếm số frame giữa lúc ngón tay chạm và lúc LED đổi. Độ trễ = frame / 60 × 1000 (ms).
Lặp 20 lần, tính trung bình + độ lệch chuẩn.

Đo ở 3 điều kiện:
- Điều khiển từ HA (cùng phòng)
- Điều khiển từ chip-tool (fabric 2)
- Điều khiển từ nút vật lý → thời gian trạng thái phản ánh lên HA

→ `data/bang-4-2-do-tre.csv`

## Thí nghiệm 3 — Multi-fabric

**Cách làm**: thêm lần lượt fabric 1, 2, 3, 4, 5... tới khi thiết bị từ chối.
Sau mỗi lần, kiểm tra tất cả fabric trước đó còn điều khiển được không.
Sau đó xoá fabric giữa, kiểm tra các fabric còn lại.

→ `data/bang-4-3-multi-fabric.csv`

## Thí nghiệm 4 — Vùng phủ sóng

**Cách làm**: đặt thiết bị ở các khoảng cách 1m, 3m, 5m, 10m, và 1 vị trí xuyên tường.
Mỗi vị trí ghi RSSI và thử 10 lệnh bật/tắt, đếm số lệnh thành công.

→ `data/bang-4-4-rssi-vung-phu.csv`

## Thí nghiệm 5 — Độ ổn định dài hạn

**Cách làm**: để hệ thống chạy liên tục ≥ 24 giờ. Mỗi giờ gửi 1 lệnh, ghi kết quả.
Theo dõi free heap để phát hiện memory leak.

Script tự động (chạy trên Ubuntu):
```bash
# gui lenh dinh ky trong 24 gio, ghi ket qua ra CSV
```

→ `data/bang-4-5-on-dinh.csv`

## Thí nghiệm 6 — Hoạt động không cần Internet

**Cách làm**: rút dây WAN của router (giữ LAN + Wi-Fi nội bộ). Thử toàn bộ chức năng.
Ghi lại chức năng nào còn, chức năng nào mất.

Dự kiến: điều khiển local vẫn chạy; chỉ mất remote access và cập nhật add-on.
Đây là luận điểm chính bảo vệ ưu thế của Matter — **quay video làm bằng chứng**.

→ ghi vào `data/bang-4-5-on-dinh.csv`

## Tài nguyên hệ thống

Chạy sau khi build:
```bash
cd ~/matter-capstone/firmware/light
idf.py size > size.txt
idf.py size-components > size-components.txt
```

→ `data/bang-4-6-tai-nguyen.csv`

## Hình ảnh cần chụp cho báo cáo

- [ ] Toàn cảnh hệ thống (Pi + ESP32 + breadboard + màn hình)
- [ ] Cận cảnh breadboard đã nối dây
- [ ] Màn hình Home Assistant hiện thiết bị Matter
- [ ] Trang device info hiện Node ID / Fabric
- [ ] Terminal chip-tool đang điều khiển thành công
- [ ] Output lệnh `read fabrics` liệt kê nhiều fabric
- [ ] OLED hiển thị số fabric
- [ ] LED sáng các màu khác nhau
- [ ] Log commissioning trong Matter Server add-on
