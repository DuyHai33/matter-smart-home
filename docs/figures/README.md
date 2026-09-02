# docs/figures — Hình dùng trong báo cáo

Thư mục này chỉ chứa **hình đã sẵn sàng chèn vào báo cáo**, đặt tên theo đúng số hình.
Ảnh thô, ảnh tư liệu, ảnh chụp nháp để ở `docs/screenshots/`.

## Bảng đối chiếu

| Tên hình trong báo cáo | File | Loại | Trạng thái |
|---|---|---|---|
| Hình 3.1 — Sơ đồ khối tổng quát hệ thống | `hinh-3-1-so-do-khoi.png` (+ `.svg`) | Vẽ | ✅ Xong 17/08 |
| Hình 3.5 — Lưu đồ quy trình commissioning thực tế | `hinh-3-5-luu-do-commissioning.png` (+ `.svg`) | Vẽ | ✅ Xong 17/08 |
| Hình 4.1 — Board ESP32-C6 đang chạy, LED sáng | `hinh-4-1-board-led-do.jpg` | Chụp | ✅ Xong 17/08 |
| Hình 4.4 — Thiết bị Matter trong Home Assistant | `hinh-4-4-ha-thiet-bi-matter.png` | Chụp | ✅ Xong 17/08 |
| Hình 4.4b — Dashboard Overview của HA | `hinh-4-4b-ha-overview.png` | Chụp | ✅ Xong 17/08 |
| Hình 4.4c — Hộp điều khiển đèn (độ sáng 47%, màu đỏ) | `hinh-4-4c-ha-dieu-khien-den.png` | Chụp | ✅ Xong 17/08 |
| Hình 4.5 — CommissionedFabrics = 5 (minh chứng multi-fabric) | `hinh-4-5-multi-fabric.png` | Chụp | ✅ Chụp lại 17/08 18:42 — đã đúng giờ VN |
| Hình 4.5b — Bảng Fabrics chi tiết, 5 entry | `hinh-4-5b-read-fabrics.png` | Chụp | ✅ Xong 17/08 18:44 |
| Hình 4.6 — LED ở ba màu đỏ / xanh lá / xanh dương | `hinh-4-6-ba-mau.jpg` | Chụp + ghép | ✅ Xong 17/08 |

## Ghi chú từng hình

**Hình 3.1 / 3.5** — bản `.svg` là bản gốc, sửa được bằng trình soạn thảo văn bản.
Bản `.png` xuất ở 1920 px chiều rộng (~336 DPI khi in khổ 145 mm), chèn thẳng vào Word.

**Hình 4.4** — đây là trang **Device info** (Settings › Devices), **không phải** Overview
dashboard. Caption trong báo cáo phải sửa cho khớp, đề xuất:
*"Thiết bị Matter trong Home Assistant — thông tin thiết bị và điều khiển"*.
Nếu muốn đúng nghĩa "dashboard" thì dùng Hình 4.4b.

**Hình 4.4c** — đáng chú ý: HA hiển thị **47%**, khớp chính xác `CurrentLevel = 120`
(120 / 254 = 47,2 %). Đây là bằng chứng đồng bộ trạng thái hai chiều, nên nhắc trong phần
bình luận của báo cáo.

**Hình 4.5** — đã chụp lại 17/08 18:42, dấu thời gian giờ VN, khớp múi giờ với Hình 4.4.
(Ảnh cũ lệch 7 tiếng do WSL chạy giờ UTC; phải đặt `TZ='Asia/Ho_Chi_Minh'` trước khi chụp.)
Nội dung: `CommissionedFabrics = 5` đọc từ **hai** controller độc lập (#2 và #3), `SupportedFabrics = 5`,
và mDNS `_matter._tcp` liệt kê đúng 5 CompressedFabricID khác nhau — trong đó `FA62912736579AB7`
là fabric của Home Assistant.

> ⚠️ **Hai con số phải giải thích trong báo cáo, không được để người chấm tự phát hiện:**
>
> 1. **`RebootCount = 10`, `UpTime = 40 s`.** Bảng 4.5 (đo ổn định 7 giờ) ghi `RebootCount = 3`
>    không đổi. 7 lần reboot chênh lệch phát sinh **sau khi phép đo kết thúc lúc 02:54**, do
>    cắm/rút USB và mở cổng serial COM6 trong lúc thao tác chụp ảnh và sao lưu flash — mở cổng
>    serial làm board reset. Đây **không phải**
>    thiết bị tự crash. Phải nói rõ điều này, nếu không hai số liệu trông như mâu thuẫn.
> 2. **`FabricIndex` của entry cuối là `18`, không phải `5`.** `FabricIndex` do thiết bị cấp
>    tăng đơn điệu và **không tái sử dụng** index của fabric đã bị xoá. Trong quá trình thử
>    nghiệm đã thêm/xoá fabric nhiều lượt (kể cả lần fabric #6 fail `No memory`), nên index
>    nhảy tới 18 trong khi số fabric đang tồn tại vẫn là 5. Không giải thích thì bị hỏi ngay.

**Hình 4.5b** — bảng `Fabrics` đọc bằng `operationalcredentials read fabrics 2 0`
**có `--fabric-filtered false`**. Không có cờ này, thiết bị chỉ trả về đúng 1 entry của chính
controller đang hỏi và ảnh mất hết giá trị chứng minh.

Chi tiết đáng dùng trong phần bình luận:

| Entry | VendorID | FabricID | NodeID | Label | Là ai |
|---|---|---|---|---|---|
| [1] | 4939 (`0x134B`) | 2 | 10 (`0x0A`) | `Home` | **Home Assistant** — khớp đúng Node ID `0x0A` ghi ở mục commissioning |
| [2]–[5] | 65521 (`0xFFF1`) | 1 | 2, 3, 4, 5 | (trống) | 4 instance `chip-tool` |

Điểm kỹ thuật nên nêu: bốn fabric `chip-tool` **có cùng `FabricID = 1`** nhưng vẫn là bốn fabric
độc lập, vì Matter phân biệt fabric bằng cặp **(RootPublicKey, FabricID)** chứ không bằng
`FabricID` đơn lẻ — và 5 `RootPublicKey` trong ảnh khác nhau hoàn toàn. `VendorID 0xFFF1` là
test vendor ID, đúng với việc firmware dùng chứng chỉ test (lý do phải bật `Test DCL` ở add-on).

**Hình 4.6** — ghép từ 3 ảnh gốc trong `docs/screenshots/4-6-led-{do,xanh-la,xanh-duong}.jpg`.
Đã crop bỏ phần nền lộn xộn và thêm nhãn toạ độ CIE cho từng màu.

## Nguyên tắc đặt tên

```
hinh-<chuong>-<so>[<hau-to>]-<mo-ta-khong-dau>.<ext>
```
Không dùng dấu cách, không dùng tiếng Việt có dấu trong tên file — tránh lỗi khi
chèn vào Word hoặc khi đẩy lên Git trên máy khác.
