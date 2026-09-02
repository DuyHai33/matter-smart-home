# 06 — BOM và hướng dẫn nối dây

> Cập nhật **17/08/2026** — viết lại sau khi xác nhận linh kiện thật đang có.
> ⚠️ **Đính chính lớn so với bản 16/08**: LED rời đang có là **LED đơn 2 chân**, KHÔNG phải
> LED RGB 4 chân cũng không phải WS2812. Toàn bộ phần "nối LED RGB rời qua 3 kênh PWM" của
> bản cũ **không áp dụng được** và đã bị gỡ.

---

## 1. BOM — Danh sách linh kiện

### 1.1 Linh kiện chính (đã có)

| # | Linh kiện | Thông số | SL | Vai trò trong hệ thống | Giá thực trả (VNĐ) |
|---|---|---|---|---|---|
| 1 | **ESP32-C6 SuperMini** | `ESP32C6FH4`, RISC-V 160 MHz, 4 MB flash, Wi-Fi 6 2.4G + BLE 5.0, USB-C | 1 | Matter End Device — chạy toàn bộ firmware | |
| 2 | **Raspberry Pi 4 Model B** | 8 GB RAM | 1 | Chạy HAOS + add-on Matter Server = Fabric #1 | |
| 3 | Nguồn USB-C cho Pi | 5V/3A | 1 | Cấp nguồn Pi | |
| 4 | Thẻ microSD | 32 GB | 1 | Chứa HAOS 18.2 | |
| 5 | Cáp micro-HDMI → HDMI | | 1 | Xem log lúc Pi boot | |
| 6 | Cáp USB-C **có dây dữ liệu** | | 1 | Nạp firmware + cấp nguồn C6 | |
| 7 | Dây mạng LAN | | 1 | Pi cắm thẳng vào router (bắt buộc cùng L2 với C6) | |
| 8 | Router FPT AX3000HV2 | có 2.4GHz + IPv6 | 1 | Hạ tầng mạng, SSID `FPT Telecom-3D02 - IoT` | |

> ⚠️ **WS2812 RGB nằm sẵn trên board ở GPIO8** → đây chính là đèn của EP1 (Extended Color Light).
> Không cần mua và không cần nối LED RGB rời. Giai đoạn 2–4 đã chạy xong bằng con LED này.

### 1.2 Linh kiện mở rộng — Giai đoạn 5 (đã có)

| # | Linh kiện | Thông số | SL | Vai trò | Giá thực trả (VNĐ) |
|---|---|---|---|---|---|
| 9 | **OLED 0.96"** | SSD1306, I2C, 128×64, địa chỉ `0x3C` | 1 | Hiển thị IP / RSSI / **số fabric** / trạng thái đèn | |
| 10 | **Nút nhấn tactile** | 6×6 mm, 4 chân | 2 | Nút 1 = toggle OnOff EP1; Nút 2 = Generic Switch EP2 | |
| 11 | **LED đơn 2 chân** | 3 mm hoặc 5 mm, chân dài = anode (+) | 1–3 | LED chỉ báo trạng thái (xem mục 3) | |
| 12 | **Điện trở 220 Ω** | 1/4 W | ≥3 | Hạn dòng cho LED đơn | |
| 13 | **Điện trở 330 Ω** | 1/4 W | ≥3 | Phương án thay thế 220 Ω (LED tối hơn chút) | |

### 1.3 Dụng cụ / vật tư lắp ráp (đã có)

| # | Vật tư | Ghi chú |
|---|---|---|
| 14 | Breadboard | Loại 400 hoặc 830 lỗ |
| 15 | Dây jumper đực-đực / đực-cái | Cần ~12 sợi |
| 16 | Đồng hồ VOM | Dùng ở bước kiểm tra — **đừng bỏ qua** |
| 17 | Mỏ hàn + thiếc | Hàn header cho SuperMini nếu board bán rời header |

### 1.4 KHÔNG cần mua thêm gì

Với danh sách trên, Giai đoạn 5 chạy được đầy đủ. Không cần LED RGB rời, không cần Border
Router, không cần USB-TTL (board dùng USB-Serial-JTAG sẵn).

---

## 2. Xác định linh kiện trước khi cắm

### 2.1 LED đơn — phân cực

```
        chân DÀI  = Anode  (+)  -> nối về phía GPIO (qua trở)
        chân NGẮN = Cathode (-)  -> nối về GND
                    │  │
                 ┌──┴──┴──┐
                 │  ╭──╮  │
                 │  ╰──╯  │   <- vành nhựa có một mặt VÁT PHẲNG
                 └────────┘      mặt vát nằm cùng phía chân ngắn (cathode)
```

Nếu chân đã bị cắt bằng nhau → dùng mặt vát để xác định, hoặc đo bằng VOM (mục 2.2).

### 2.2 Đo LED bằng VOM — 20 giây (nên làm, vì nó cho biết luôn LED có sáng nổi ở 3.3V không)

Vặn VOM về **chế độ diode** (ký hiệu ▷|):

| Kết quả | Nghĩa |
|---|---|
| Que **đỏ** chạm chân dài, que đen chạm chân ngắn → hiện số **1.6–3.4** | Đúng chiều. Số đó là **Vf** — ghi lại. |
| Đảo que → hiện `OL` / `1.` | Bình thường (diode chặn chiều ngược) |
| Cả 2 chiều đều `OL` | LED đứt / VOM yếu pin |
| Cả 2 chiều đều ~0 | LED chập, bỏ |

**Vf quyết định LED có sáng được từ GPIO 3.3V hay không:**

| Màu LED | Vf điển hình | Dòng với 220 Ω | Dòng với 330 Ω | Kết luận |
|---|---|---|---|---|
| Đỏ | ~2.0 V | 5.9 mA | 3.9 mA | Sáng tốt |
| Vàng / Cam | ~2.1 V | 5.5 mA | 3.6 mA | Sáng tốt |
| Xanh lá thường | ~2.1 V | 5.5 mA | 3.6 mA | Sáng tốt |
| Xanh dương / Trắng | ~3.0 V | 1.4 mA | 0.9 mA | **Mờ** — dùng 220 Ω, đừng dùng 330 Ω |
| Bất kỳ, Vf đo được **> 3.2 V** | | ~0 | ~0 | **Không sáng nổi từ 3.3V** — đổi LED khác |

Công thức: `I = (3.3 V − Vf) / R`. ESP32-C6 lái được 20 mA/chân, nên 220 Ω là an toàn tuyệt đối.

👉 **Chốt: dùng 220 Ω cho tất cả LED.** Cặp 330 Ω để dành làm dự phòng.

### 2.3 Đọc trở — dùng VOM cho chắc

Đừng đoán vòng màu, vặn VOM thang **200 Ω** đo trực tiếp: ra ~220 hoặc ~330.
(Tham khảo vòng màu: 220 Ω = Đỏ-Đỏ-Nâu-Nhũ vàng · 330 Ω = Cam-Cam-Nâu-Nhũ vàng)

### 2.4 Nút tactile 4 chân — tìm đúng cặp

4 chân của nút thực chất là **2 cặp đã nối sẵn bên trong**. Cắm nhầm 2 chân cùng cặp →
mạch thông vĩnh viễn, nút vô tác dụng.

**Cách chống nhầm chắc chắn nhất: cắm nút NGANG rãnh giữa breadboard**, rồi lấy
**1 chân bên trái rãnh + 1 chân bên phải rãnh**. Hai chân đó luôn thuộc 2 cặp khác nhau.

Muốn kiểm chứng: VOM thang thông mạch, chạm 2 chân → không kêu, nhấn nút → kêu. Đúng cặp.

---

## 3. LED đơn dùng để làm gì trong đồ án

EP1 đã có WS2812 onboard rồi, nên LED đơn **không phải** đèn chính. Dùng làm **LED chỉ báo** —
vừa hữu ích khi debug, vừa dễ chụp ảnh minh hoạ cho báo cáo hơn con WS2812 SMD bé xíu:

**Chốt 17/08/2026: dùng đủ 3 LED — xanh lá, đỏ, xanh dương.**

| LED | Màu | GPIO | Ý nghĩa |
|---|---|---|---|
| LED 1 | **Xanh lá** | GPIO0 | **Trạng thái mạng**: sáng = Wi-Fi connected và đã commissioned |
| LED 2 | **Đỏ** | GPIO1 | **Gương OnOff của EP1**: bật/tắt đúng theo lệnh Matter (nhìn rõ hơn WS2812) |
| LED 3 | **Xanh dương** | GPIO2 | Nháy 1 cái mỗi khi nhận lệnh từ fabric bất kỳ — minh hoạ multi-admin |

> ⚠️ **Gán màu không tuỳ ý — căn theo Vf.** LED xanh dương có Vf ~3.0–3.2 V, qua 220 Ω từ
> nguồn 3.3 V chỉ ra **~1.4 mA**, mờ hẳn so với đỏ và xanh lá (~5.5 mA). Vì vậy nó nhận vai
> trò *nháy* — mắt vẫn bắt được nhịp nháy dù mờ. LED đỏ sáng nhất giữ vai trò gương OnOff,
> vì đó là LED dùng để chụp ảnh và demo.
>
> Nếu đo được **Vf > 3.2 V** thì con đó không sáng nổi từ GPIO 3.3 V → bỏ, chỉ dùng 2 LED.

---

## 4. Bản đồ GPIO

Pinout thật của board: `docs/screenshots/esp32-c6-supermini-pinout.jpg`.

```
              cạnh trái                                cạnh phải
            ┌──────────┐                             ┌──────────┐
       TX   │● (GP16)  │      ┌─────────┐            │  5V     ●│   ra từ USB
       RX   │● (GP17)  │      │  USB-C  │            │  GND    ●│   ← GND chính
      GP0   │●  LED1 xanh lá │ └────────┘            │  3V3    ●│   ← nguồn OLED
      GP1   │●  LED2 đỏ      │                       │  GP20   ●│   Nút 1
      GP2   │●  LED3 xanh dương │     ESP32-C6       │  GP19   ●│   Nút 2
      GP3   │●         │        SuperMini            │  GP18   ●│   Nút 3
      GP4   │● strap   │       26.2 × 18.2 mm        │  GP15   ●│ strap + LED xanh
      GP5   │● strap   │                             │  GP14   ●│
      GP6   │●  SDA    │                             │  GP9    ●│ BOOT
      GP7   │●  SCL    │                             │  GP8    ●│ ← WS2812, ĐỪNG ĐỘNG
            └──────────┘                             └──────────┘

         ── pad rời ở mép dưới board (KHÔNG dùng, xem ghi chú) ──
              GP22 · GP23 (mép trái) · GP12 · GP13 · GP21 (mép phải)
```

> ⚠️ **Đính chính 17/08/2026 — vì sao OLED chuyển sang GP6/GP7.**
> `GP22`, `GP23`, `GP12`, `GP13`, `GP21` **không nằm trên 2 hàng header 2.54 mm**, chúng là
> **pad rời ở mép dưới board**, sát vùng antenna (xem `docs/screenshots/esp32-c6-supermini-pinout.jpg`).
> Muốn dùng phải hàn dây trần vào pad → dễ bong pad, dễ chạm, và nếu chết board là **mất
> luôn 5 fabric** đang giữ trong NVS. Không đáng đánh cược.
>
> Không cần hàn, vì **I2C của ESP32-C6 đi qua GPIO matrix** — SDA/SCL gán được vào bất kỳ
> chân nào (khác AVR/STM32 nơi I2C cố định chân). Con số 22/23 ở bản doc cũ chỉ là lựa chọn
> tuỳ ý, không phải ràng buộc phần cứng. Firmware chưa viết dòng code OLED nào nên đổi
> chân là miễn phí.
>
> **Chốt: SDA = GP6, SCL = GP7. Nút 2 chuyển GP21 → GP19.** Cả 3 chân đều nằm trên header.

> Sơ đồ nhà sản xuất in nhãn `GP8` ở cả hai mép. Dù nó có ra chân hay không:
> **tuyệt đối không nối gì vào GP8** — đó là dây DIN của WS2812 onboard đang chạy EP1.

### Chân dùng được an toàn — CÓ SẴN TRÊN HEADER
`GPIO0, 1, 2, 3, 6, 7, 14, 18, 19, 20`

### Chân cấm / cần tránh

| GPIO | Lý do |
|---|---|
| **8** | WS2812 onboard — EP1 đang dùng |
| 9 | Strapping + nút BOOT |
| 12, 13 | USB-Serial-JTAG D−/D+ — **dùng là mất cổng nạp** |
| 15 | Strapping + LED xanh onboard |
| 4, 5 | Strapping pin của ESP32-C6 |
| 16, 17 | UART0 TX/RX — log console |
| **21, 22, 23** | Pad rời ở mép dưới, không có header — phải hàn, không dùng (xem ghi chú mục 4) |

---

## 5. Bảng nối dây

| Linh kiện | Chân linh kiện | Nối tới | Qua trở |
|---|---|---|---|
| WS2812 onboard | — | **GPIO8 (có sẵn trên board)** | — |
| OLED SSD1306 | VCC | **3V3** | — |
| | GND | **GND** | — |
| | SDA | **GPIO6** | — |
| | SCL | **GPIO7** | — |
| LED 1 **xanh lá** | chân dài (+) | **GPIO0** | **220 Ω** |
| | chân ngắn (−) | **GND** | — |
| LED 2 **đỏ** | chân dài (+) | **GPIO1** | **220 Ω** |
| | chân ngắn (−) | **GND** | — |
| LED 3 **xanh dương** | chân dài (+) | **GPIO2** | **220 Ω** |
| | chân ngắn (−) | **GND** | — |
| Nút 1 | 1 chân | **GPIO20** | — |
| | chân đối diện | **GND** | — |
| Nút 2 | 1 chân | **GPIO19** | — |
| | chân đối diện | **GND** | — |
| Nút 3 | 1 chân | **GPIO18** | — |
| | chân đối diện | **GND** | — |

Ba nút **không cần trở kéo lên ngoài** — dùng pull-up nội của ESP32-C6
(`gpio_pullup_en`), nhấn = mức 0.

### Chức năng 3 nút

| Nút | GPIO | Chức năng |
|---|---|---|
| Nút 1 | GPIO20 | Toggle OnOff EP1 — bật/tắt đèn, trạng thái đồng bộ về cả 5 fabric |
| Nút 2 | GPIO19 | Generic Switch EP2 — phát event Matter (`InitialPress` / `ShortRelease`) |
| Nút 3 | GPIO18 | Đổi trang hiển thị OLED |

> ⚠️ **TUYỆT ĐỐI KHÔNG gán nút nào làm factory reset / decommission.** Firmware example của
> esp-matter **đã có sẵn** chức năng đó trên nút **BOOT (GP9)** onboard. Thêm một nút reset nữa
> trên breadboard thì một lần nhấn nhầm là **xoá sạch 5 fabric** — mất đúng điểm nhấn của đồ án
> và phải commission lại 5 lượt. Không đáng đánh cược, nhất là sát deadline.

### Sơ đồ tổng

```
   ESP32-C6 SuperMini                      Breadboard
   ┌─────────────────┐
   │ GP8  ●  WS2812 onboard (EP1) — không nối dây
   │
   │ 3V3  ●──────────────────────────►  ray (+)  ──► VCC OLED
   │ GND  ●──────────────────────────►  ray (−)  ──┬─► GND OLED
   │                                                ├─► chân ngắn LED1 xanh lá
   │                                                ├─► chân ngắn LED2 đỏ
   │                                                ├─► chân ngắn LED3 xanh dương
   │                                                ├─► Nút 1 (chân đối diện)
   │                                                ├─► Nút 2 (chân đối diện)
   │                                                └─► Nút 3 (chân đối diện)
   │
   │ GP0  ●───[220Ω]───► chân dài LED 1  XANH LÁ
   │ GP1  ●───[220Ω]───► chân dài LED 2  ĐỎ
   │ GP2  ●───[220Ω]───► chân dài LED 3  XANH DƯƠNG
   │
   │ GP6  ●────────────► SDA (OLED)
   │ GP7  ●────────────► SCL (OLED)
   │
   │ GP20 ●────────────► Nút 1  (toggle đèn)
   │ GP19 ●────────────► Nút 2  (Generic Switch EP2)
   │ GP18 ●────────────► Nút 3  (đổi trang OLED)
   └─────────────────┘
```

---

## 5b. Sơ đồ cắm theo đúng toạ độ breadboard (bản đang lắp thật)

> Chốt 17/08/2026 sau khi đối chiếu ảnh `docs/screenshots/Test Thực tế chưa cắm.jpg`.
> Breadboard 400 lỗ, cột **a b c d e ‖ f g h i j**, hàng **1–30**.
> Board ESP32-C6 cắm chiếm **hàng 30 → 21**, cổng **USB-C ở phía hàng 30**.
> Ký hiệu `(hàng, cột)`, ví dụ `(28, b)` = hàng 28 cột b.

### Bản đồ hàng ↔ chân

| Hàng | Cạnh trái (a–e) | Cạnh phải (f–j) |
|---|---|---|
| 30 | TX | 5V |
| 29 | RX | **GND** → ray (−) |
| 28 | **GP0** → LED xanh lá | **3V3** → ray (+) |
| 27 | **GP1** → LED đỏ | **GP20** → Nút 1 |
| 26 | **GP2** → LED xanh dương | **GP19** → Nút 2 |
| 25 | GP3 *(dự phòng)* | **GP18** → Nút 3 |
| 24 | GP4 ⛔ strap | GP15 ⛔ strap |
| 23 | GP5 ⛔ strap | GP14 *(dự phòng)* |
| 22 | **GP6** → SDA | GP9 ⛔ BOOT |
| 21 | **GP7** → SCL | GP8 ⛔ WS2812 |

Vùng còn trống: **hàng 9–20** dành cho 3 LED · **hàng 1–8** đã cắm 3 nút.

> Sơ đồ dưới đây đúng **bất kể board đang cắm ở cột nào** (c/h hay b/g), vì 5 lỗ `a b c d e`
> của một hàng là **cùng một node**, và `f g h i j` cũng vậy. Miễn hai hàng chân của board nằm
> ở hai phía rãnh giữa, thì cắm jumper vào cột `a`/`b` là nối được chân cạnh trái, cắm vào cột
> `i`/`j` là nối được chân cạnh phải. Chỉ cần **số hàng** đúng.

### ⚠️ Nối 2 ray nguồn — lỗi dễ quên nhất

Breadboard 400 lỗ có **4 ray độc lập**: ray (−) bên trái **KHÔNG** nối sẵn với ray (−) bên phải.
Board cấp nguồn từ cạnh phải, còn OLED và 3 nút lại ở bên trái. Thiếu 2 jumper cầu ray thì nửa
mạch không có GND — và VOM đo từng nhánh riêng vẫn cho kết quả "đúng", nên rất mất thời gian
để tìm ra.

### Nhóm 1 — Nguồn (4 dây, làm trước tiên)

```
(29, i)  GND board  ──►  ray (−) PHAI
(28, i)  3V3 board  ──►  ray (+) PHAI
ray (−) PHAI  ◄──────►  ray (−) TRAI      <- cau ray, dung quen
ray (+) PHAI  ◄──────►  ray (+) TRAI      <- cau ray, dung quen
```

### Nhóm 2 — OLED (4 dây)

```
VCC ──► ray (+) TRAI
GND ──► ray (−) TRAI
SDA ──► (22, a)      = GP6
SCL ──► (21, a)      = GP7
```

### Nhóm 3 — 3 LED

Trở bắc **ngang rãnh giữa** (cột e → cột f cùng hàng). Đây là cách tiết kiệm chỗ nhất: mỗi LED
chỉ ăn 2 hàng, và cathode về được ray (−) phải — cùng ray với GND của board.

```
LED XANH LA  — GP0
  (28, b) ──jumper──► (10, a)
  (10, e) ──[220Ω]──► (10, f)        <- tro bac ngang ranh
  (10, g) ◄── chan DAI  (+) LED
  (11, g) ◄── chan NGAN (−) LED
  (11, i) ──jumper──► ray (−) PHAI

LED DO  — GP1
  (27, b) ──jumper──► (14, a)
  (14, e) ──[220Ω]──► (14, f)
  (14, g) ◄── chan DAI  (+)
  (15, g) ◄── chan NGAN (−)
  (15, i) ──jumper──► ray (−) PHAI

LED XANH DUONG  — GP2
  (26, b) ──jumper──► (18, a)
  (18, e) ──[220Ω]──► (18, f)
  (18, g) ◄── chan DAI  (+)
  (19, g) ◄── chan NGAN (−)
  (19, i) ──jumper──► ray (−) PHAI
```

Hai chân LED cách nhau 2,54 mm = **đúng 1 hàng**, nên anode hàng N / cathode hàng N+1 vừa khít,
không phải uốn chân.

⚠️ Hai đầu trở phải nằm ở **hai node khác nhau**. Cắm cả hai đầu vào cùng một hàng cùng một phía
rãnh là **nối tắt trở** → LED ăn thẳng 3,3 V và cháy.

### Nhóm 4 — 3 nút (đã cắm ngang rãnh, chỉ đi dây)

Lấy chân **phía cột f–j** nối lên GPIO, để dây không phải vắt qua thân board.

```
Nut 1: chan phia f/g ──► (27, i) = GP20   ·   chan phia d/e ──► ray (−) TRAI
Nut 2: chan phia f/g ──► (26, i) = GP19   ·   chan phia d/e ──► ray (−) TRAI
Nut 3: chan phia f/g ──► (25, i) = GP18   ·   chan phia d/e ──► ray (−) TRAI
```

Hai chân dùng của mỗi nút phải **chéo nhau qua rãnh**. Cùng phía rãnh = 2 chân cùng cặp đã nối
sẵn bên trong → nút thông mạch vĩnh viễn, vô tác dụng.

### Vật tư cần cho cách cắm này

Khoảng **20 sợi jumper**. Mục 1.3 ghi có ~12 sợi → **đếm lại trước khi bắt đầu**. Thiếu thì cắt
chân trở dư làm cầu nối cho các đoạn ngắn (cathode LED → ray).

---

## 6. Trình tự cắm — làm đúng thứ tự này

### Bước 0 — RÚT USB
Không cắm/tháo linh kiện khi board đang có điện. Đây là nguyên nhân chết chip phổ biến nhất.

### Bước 1 — Hàn header (bỏ qua nếu board đã có header)
SuperMini hay bán kèm header rời. Hàn 2 hàng chân đực xuống mặt dưới, mũi hàn chạm 1–2 giây,
không quá 3 giây mỗi mối. Hàn xong dùng VOM thang thông mạch kiểm 2 mối kề nhau **không** dính.

### Bước 2 — Cắm board lên breadboard
Cắm **ngang rãnh giữa**, chip hướng lên. Board rộng 18.2 mm ≈ 7 lỗ:
- Với breadboard chuẩn (hàng A…E · rãnh · F…J): hàng chân trái vào **cột C**, hàng chân phải vào **cột H**.
- Còn lại cột A, B (bên trái) và I, J (bên phải) để cắm jumper.

> Chỉ còn 2 lỗ mỗi bên nên khá chật. Mẹo: cắm hết jumper vào các hàng đó **trước**, rồi mới ấn board xuống.

### Bước 3 — Dựng ray nguồn
- Jumper từ chân **3V3** của board → ray **(+)** của breadboard
- Jumper từ chân **GND** của board → ray **(−)** của breadboard

⚠️ Dùng **3V3**, không dùng 5V. OLED và LED đều chạy 3.3V. Cấp 5V vào chân I/O sẽ hỏng chip.

### Bước 4 — Cắm OLED
VCC → ray (+) · GND → ray (−) · SDA → **GP6** · SCL → **GP7**.
Kiểm lại thứ tự chân **in trên chính module** — có module là `GND VCC SCL SDA`, có module là
`VCC GND SCL SDA`. **Cắm ngược VCC/GND là cháy OLED ngay.**

### Bước 5 — Cắm LED + trở
Với mỗi LED:
1. Cắm **trở 220 Ω** bắc qua rãnh: một đầu vào hàng nối tới GPIO, đầu kia sang một hàng trống.
2. Cắm **chân dài** của LED vào hàng trống đó (chung hàng với đầu trở).
3. Cắm **chân ngắn** của LED vào ray **(−)**.
4. Jumper từ GPIO tương ứng (GP0 / GP1 / GP2) tới đầu trở còn lại.

Trở đặt phía nào của LED cũng được về mặt điện, nhưng đặt ở phía GPIO như trên thì nhìn sơ đồ rõ hơn.

### Bước 6 — Cắm 3 nút
Cắm nút **ngang rãnh giữa**. Lấy 1 chân bên trái rãnh → GPIO (GP20 / GP19 / GP18),
1 chân bên phải rãnh → ray (−).

### Bước 7 — KIỂM TRA BẰNG VOM TRƯỚC KHI CẤP NGUỒN

| Phép đo | Kỳ vọng | Nếu sai |
|---|---|---|
| Thang thông mạch: ray (+) ↔ ray (−) | **KHÔNG kêu** | Có chập nguồn — rà lại toàn bộ, đừng cắm USB |
| Thang thông mạch: GP0 ↔ ray (−) | không kêu (có LED chặn) | Kêu = LED cắm thiếu trở hoặc chạm |
| Thang thông mạch: chân GND board ↔ ray (−) | **Kêu** | Jumper GND lỏng |
| Thang thông mạch: chân 3V3 board ↔ ray (+) | **Kêu** | Jumper 3V3 lỏng |
| Nhấn Nút 1: GP20 ↔ ray (−) | không kêu → **kêu** khi nhấn | Cắm nhầm 2 chân cùng cặp, xoay nút 90° |
| Nhấn Nút 2: GP19 ↔ ray (−) | không kêu → **kêu** khi nhấn | Như trên |
| Nhấn Nút 3: GP18 ↔ ray (−) | không kêu → **kêu** khi nhấn | Như trên |
| Thang thông mạch: GP6 ↔ GP7 | **KHÔNG kêu** | SDA/SCL dính nhau, rà lại jumper |
| Thang thông mạch: GP20 ↔ GP19, GP19 ↔ GP18 | **KHÔNG kêu** (cả khi nhấn) | Hai nút cắm chồng hàng, xê ra |

### Bước 8 — Cấp nguồn
Cắm USB. Trong 2 giây đầu: sờ nhẹ mặt chip C6 và module OLED — **ấm nhẹ là bình thường, nóng
rát là rút ngay** và quay lại bước 7.

---

## 7. Test từng khối trước khi đụng vào firmware Matter

Test riêng lẻ trước, ghép sau. Ghép thẳng vào firmware Matter rồi mới debug là cách mất nhiều giờ nhất.

### 7.1 Quét I2C tìm OLED
```bash
source ~/esp/esp-idf/export.sh
cp -r $IDF_PATH/examples/peripherals/i2c/i2c_tools ~/i2c-test && cd ~/i2c-test
idf.py set-target esp32c6 && idf.py build
```
Nạp từ **PowerShell Windows** (WSL không thấy USB):
```powershell
$b = "\\wsl.localhost\Ubuntu\home\duyhai\i2c-test\build"
py -m esptool --chip esp32c6 --port COM6 --baud 460800 write-flash 0x0 "$b\bootloader\bootloader.bin" 0x8000 "$b\partition_table\partition-table.bin" 0x10000 "$b\i2c-tools.bin"
```
Trong monitor gõ: `i2cconfig --sda=6 --scl=7` rồi `i2cdetect`.
→ Phải thấy **`3c`**. Không thấy: đảo SDA/SCL, kiểm nguồn OLED, thử địa chỉ `0x3D`.

### 7.2 Test LED và nút
Viết một app `blink` nhỏ nháy GP0/GP1/GP2 và in mức logic GP20/GP19/GP18. Xác nhận:
- Cả 3 LED nháy đúng nhịp → dây LED + trở OK
- Nhấn từng nút → log đổi từ `1` sang `0` → nút + pull-up OK
- So độ sáng 3 LED: xanh dương mờ hơn rõ rệt là **bình thường** (Vf cao, xem mục 3)

### 7.3 ⚠️ CẢNH BÁO trước khi nạp lại firmware Matter

Board hiện đang giữ **5 fabric** (HA + 4 chip-tool) — điểm nhấn của đồ án.

Bố cục flash của firmware Matter (`firmware/light/partitions.csv`) — **NVS nằm ở `0x10000`**:

```
nvs,   data, nvs,   0x10000, 0xC000     <- credentials của 5 fabric nằm ở đây
ota_0, app,  ota_0, 0x20000, 0x1E0000   <- app Matter
```

| Việc | Ảnh hưởng tới 5 fabric |
|---|---|
| `write-flash` **chỉ app Matter** (`0x20000 light.bin`) | NVS không bị chạm → **giữ được** fabric |
| `idf.py erase-flash` | **Xoá sạch NVS → mất cả 5 fabric**, phải commission lại từ đầu |
| Nạp firmware test ở mục 7.1 / 7.2 | ⚠️ **MẤT CẢ 5 FABRIC** — xem cảnh báo dưới |

> ⚠️ **Đính chính 17/08/2026 — app test xoá luôn fabric, không chỉ xoá app.**
> App ESP-IDF thường dùng **partition table mặc định**, app đặt ở `0x10000` — **trùng đúng vào
> vùng NVS của firmware Matter**. Nạp app test ở mục 7.1/7.2 sẽ ghi đè `0x10000..~0x3A000`,
> tức **xoá sạch credentials của cả 5 fabric**, chứ không chỉ mất app Matter như bản doc cũ ghi.
> Đã gặp thật khi test phần cứng 17/08.
>
> **Bắt buộc backup toàn bộ flash trước khi nạp bất kỳ app test nào:**
> ```powershell
> py -m esptool --chip esp32c6 --port COM6 --baud 921600 read-flash 0 0x400000 backup-flash-4MB-5fabric.bin
> ```
> Khôi phục sau khi test xong:
> ```powershell
> py -m esptool --chip esp32c6 --port COM6 --baud 921600 write-flash 0 backup-flash-4MB-5fabric.bin
> ```
> Đã kiểm chứng 17/08: khôi phục xong chip-tool đọc lại `CommissionedFabrics: 5`, không mất fabric nào.

Nên: **chụp xong toàn bộ screenshot còn thiếu** (`read fabrics`, `commissioned-fabrics`,
HA dashboard) **rồi mới** động vào firmware. Số liệu đo đạc trong `data/` thì đã an toàn, đã lưu file.

---

## 8. Nội dung hiển thị trên OLED (128×64)

```
┌────────────────────────┐
│ MATTER DEVICE          │  tiêu đề
│ WiFi: ...-IoT   -22dBm │  SSID + RSSI
│ IP: 192.168.1.39       │  IPv4
│ Fabrics: 5             │  ← số fabric, minh hoạ multi-admin
│ Light: ON  Lvl:254     │  trạng thái EP1
│ XY: 45913,19615        │  màu hiện tại (firmware chỉ hỗ trợ XY)
└────────────────────────┘
```

Dòng `Fabrics` là chi tiết đắt giá khi demo — người chấm thấy con số tăng lên sau mỗi lần
commissioning từ controller mới.

**Nút 3 (GP18) đổi trang** — 3 trang luân phiên:

| Trang | Nội dung |
|---|---|
| 1 | Mặc định như trên: SSID, RSSI, IP, số fabric, trạng thái đèn |
| 2 | Liệt kê từng fabric: `FabricIndex`, `NodeID`, `VendorID` — bản OLED của Hình 4.5b |
| 3 | Bộ đếm: số lệnh nhận được từ mỗi fabric kể từ lúc boot — minh hoạ multi-admin động |

Trang 2 là trang đáng demo nhất: người chấm thấy ngay 5 fabric ngay trên thiết bị, không cần
mở terminal.

---

## 9. Cấp nguồn

- Cắm USB từ máy tính: board tự lấy nguồn USB, thừa sức cho OLED (~20 mA) + 3 LED (~18 mA).
- Chạy độc lập: sạc điện thoại 5V/1A qua cổng USB-C của board.
- **Không** cấp nguồn ngoài vào chân 3V3 khi vẫn đang cắm USB.

---

## 10. Checkpoint

- [x] Xác nhận board có WS2812 onboard ở GPIO8 → dùng cho EP1
- [x] Xác nhận flash 4 MB (`ESP32C6FH4`)
- [x] **Xác nhận LED rời là LED đơn 2 chân** (17/08/2026) → không làm EP1, chuyển sang vai trò LED chỉ báo
- [x] Đã có trở 220 Ω và 330 Ω
- [x] Đo Vf từng LED bằng VOM, phân loại trở 220/330 Ω — đã kiểm 17/08
- [x] **Xác nhận GP21/22/23 là pad rời không có header** (17/08/2026) → OLED chuyển sang GP6/GP7, Nút 2 sang GP19
- [x] **Chốt cấu hình 3 LED (xanh lá / đỏ / xanh dương) + 3 nút (GP20/GP19/GP18)** — 17/08/2026
- [x] Hàn header cho SuperMini — xong 17/08
- [x] Đối chiếu chữ in trên board + thứ tự chân OLED — đã kiểm 17/08
- [x] Cắm theo mục 6 — xong 17/08
- [x] **I2C scan thấy `0x3c`** — 17/08/2026, ổn định 6 lần quét / 3 lần boot
- [x] **Test riêng LED + nút bằng app `~/hw-test`** — 17/08/2026, cả 3 LED và cả 3 nút ĐẠT.
      Lần cắm đầu 3 nút bị nối tắt (lấy nhầm 2 chân cùng cặp), sửa bằng cách xoay nút 90°.
- [x] **Khôi phục firmware Matter + 5 fabric từ backup sau khi test** — 17/08/2026
- [ ] Chụp ảnh mạch hoàn chỉnh → hình cho Chương III/IV
