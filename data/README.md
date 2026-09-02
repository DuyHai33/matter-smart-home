# data/ — Bảng số liệu Chương IV

## ⚠️ NGUYÊN TẮC

Chỉ điền số đo thật từ phần cứng.

Lời cam đoan trong báo cáo đã ghi:
> "Toàn bộ số liệu thực nghiệm trình bày trong báo cáo được thu thập từ hệ thống do em tự
> thiết kế và thi công, không sao chép hay chỉnh sửa cho phù hợp với kết quả mong muốn."

Chỉ tiêu nào không đo được thì để trống và ghi rõ lý do ngay trong file, không điền số cho đủ
bảng. Ba dòng free heap của `bang-4-6` là ví dụ: firmware không có attribute `CurrentHeapFree`
nên để trống kèm lý do, thay vì ước lượng.

Mỗi file CSV có phần ghi chú phương pháp đo ở cuối, bắt đầu bằng `#`. Đọc phần đó trước khi
dùng số liệu — một số bảng có điểm bất thường đã được giải thích sẵn.

## Danh sách bảng

| File | Nội dung | Tương ứng mục |
|---|---|---|
| `bang-4-1-commissioning.csv` | Độ tin cậy và thời gian commissioning | Chương IV |
| `bang-4-2-do-tre.csv` | Độ trễ điều khiển end-to-end | Chương IV |
| `bang-4-3-multi-fabric.csv` | Kết quả thử nghiệm đa fabric | Chương IV |
| `bang-4-4-rssi-vung-phu.csv` | RSSI và tỉ lệ thành công theo khoảng cách | Chương IV |
| `bang-4-5-on-dinh.csv` | Độ ổn định dài hạn + hoạt động offline | Chương IV |
| `bang-4-6-tai-nguyen.csv` | Dung lượng flash, RAM sử dụng | Chương IV |

## Cách mở

Mở bằng LibreOffice Calc hoặc VS Code (extension Rainbow CSV).
Khi đưa vào Word, dùng Insert → Table → Convert Text to Table.
