# 05 — Multi-fabric / Multi-admin (ĐIỂM NHẤN CỦA ĐỒ ÁN)

Đây là phần chứng minh giá trị của Matter mà giao thức tự chế (MQTT/HTTP) không làm được:
**một thiết bị, nhiều nhà quản trị độc lập, không nhà nào phụ thuộc nhà nào.**

## Khái niệm cần nắm để viết báo cáo

| Thuật ngữ | Nghĩa |
|---|---|
| Fabric | Một "miền tin cậy" gồm các node dùng chung một Root CA. Mỗi hệ sinh thái = 1 fabric |
| Fabric Index | Chỉ số fabric cục bộ trên thiết bị (1, 2, 3...) |
| NOC | Node Operational Certificate — chứng chỉ thiết bị được cấp khi vào fabric |
| Multi-admin | Cơ chế cho phép thiết bị thuộc nhiều fabric cùng lúc |
| Commissioning Window | Cửa sổ thời gian thiết bị cho phép admin mới ghép vào |

Thiết bị Matter chuẩn phải hỗ trợ tối thiểu 5 fabric.

## Vì sao dùng chip-tool thay vì Apple Home / Google Home

Apple Home cần HomePod hoặc Apple TV làm hub; Google Home cần Nest Hub/Nest Mini.
Không có sẵn, mua thì tốn tiền và trễ tiến độ.

**chip-tool là controller tham chiếu chính thức của CSA** (trong repo connectedhomeip).
Mỗi instance chip-tool với `--storage-directory` riêng sẽ tự sinh Root CA riêng
→ **là fabric hoàn toàn độc lập**, đúng về mặt kỹ thuật y như một hệ sinh thái thương mại.

Trong báo cáo viết rõ: "sử dụng controller tham chiếu chip-tool để mô phỏng các hệ sinh thái
độc lập, do hạn chế về phần cứng hub thương mại". Trung thực và vẫn đúng kỹ thuật.

## Quy trình thêm fabric thứ 2

### B1 — Chuẩn bị thư mục lưu trữ riêng

```bash
mkdir -p ~/matter-fabrics/fabric2 ~/matter-fabrics/fabric3
export CHIPTOOL=~/esp/esp-matter/connectedhomeip/connectedhomeip/out/host/chip-tool
```

### B2 — Mở commissioning window từ fabric #1 (Home Assistant)

Trong HA: **Settings → Devices & Services → Matter → chọn thiết bị →
menu 3 chấm → "Share device"** → HA trả về một **mã pairing mới** (khác mã ban đầu).

Mã này chỉ sống 15 phút. Ghi lại ngay.

### B3 — Commission vào fabric #2

```bash
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 \
  pairing code 2 <MA-PAIRING-MOI>
```

`2` ở đây là Node ID trong fabric #2, không liên quan tới Node ID bên HA.

### B4 — Kiểm tra điều khiển từ fabric #2

```bash
# Bật đèn
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 onoff on 2 1

# Tắt
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 onoff off 2 1

# Đọc trạng thái
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 onoff read on-off 2 1

# Đổi độ sáng (0-254)
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 levelcontrol move-to-level 128 0 0 0 2 1

# Đổi màu (Hue 0-254, Saturation 0-254)
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 colorcontrol move-to-hue-and-saturation 200 254 0 0 0 2 1
```

### B5 — Xem danh sách fabric trên thiết bị

```bash
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 \
  operationalcredentials read fabrics 2 0
```

Output liệt kê từng fabric với `fabricIndex`, `label`, `vendorID`.
**Screenshot cái này — nó là bằng chứng trực tiếp cho multi-admin trong báo cáo.**

### B6 — Lặp lại cho fabric #3

Mở commissioning window lần nữa (từ HA hoặc từ chip-tool fabric #2):

```bash
# Mở window từ fabric 2, timeout 300s
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric2 \
  pairing open-commissioning-window 2 1 300 2000 3841
```

Rồi commission vào fabric 3:
```bash
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric3 \
  pairing code 3 <MA-MOI>
```

## Kịch bản đo cho Chương IV

| Thí nghiệm | Cách làm | Bảng |
|---|---|---|
| Số fabric tối đa | Thêm fabric tới khi thiết bị từ chối | `data/bang-4-3-multi-fabric.csv` |
| Đồng bộ trạng thái | Bật từ fabric 1 → đọc attribute từ fabric 2, đo độ trễ | `data/bang-4-2-do-tre.csv` |
| Độc lập fabric | Xoá fabric 2 (`removefabric`), kiểm tra fabric 1 và 3 vẫn chạy | `data/bang-4-3-multi-fabric.csv` |
| Hoạt động offline | Rút Internet router, thử điều khiển | `data/bang-4-5-on-dinh.csv` |

Xoá một fabric:
```bash
$CHIPTOOL --storage-directory ~/matter-fabrics/fabric3 \
  operationalcredentials remove-fabric <fabricIndex> 3 0
```

## Lưu ý

- Mỗi lần commission sinh CA mới → **đừng xoá thư mục `~/matter-fabrics/fabric*`**, xoá là mất fabric
- chip-tool cần avahi chạy và IPv6 hoạt động
- Nếu chip-tool treo ở `Discovering`, kiểm tra firewall: `sudo ufw status`
