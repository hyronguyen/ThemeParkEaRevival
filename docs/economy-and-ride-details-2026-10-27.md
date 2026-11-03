# Gameplay và giao diện ride — 27/10/2026 (lượt 2)

## Cách chơi

- Bấm công trình để mở chi tiết; Esc hoặc X đóng. Bấm tay nhấn/thanh vui hoặc
  bấm lại chính công trình để chơi. Ride cần mở và có khách.
- Mỗi lượt dài 7 giây. Thanh 0–100%, vùng vui 30–80%; dưới 30 buồn, trên 80
  quá tải (đỏ). Thanh giảm 7 điểm/giây; mỗi nhấn thêm 8 điểm, tăng tốc chuyển động.
  Muốn thắng cần kết thúc trong vùng vui và ở vùng này ít nhất 3,5 giây.
- Thắng thêm $80 và 5 flag vào phần thưởng chờ thu, tăng streak một bậc.
  Thua đặt streak về 0. Mỗi bậc cộng 5% tiền thu, tối đa +50%.
- Ride mở tích lợi nhuận mỗi 30 giây: $30 + $10/cấp cổng và 2 + cấp cổng flag.
  Bấm icon tiền nổi trên ride để nhận. Tiền/flag hiện hiệu ứng bay lên; thu lần hai
  không phát thưởng lại. Mỗi 400 flag tăng rank; lên mỗi rank được 1 ticket.
- UPGRADE nâng cổng bằng $100 × (cấp hiện tại + 1); nút ticket cạnh bên đổi 1 ticket
  lấy một cấp. Cổng có 4 hình từ cấp 0 tới 3, không quay vòng về cấp đầu.
- SELL hoàn lại nửa giá xây cộng $50/cấp cổng; khách trên ride được đưa về đường,
  khách đang đi tới được giải phóng, các tham chiếu ride còn lại được cập nhật.
- Giá xây tạm: diện tích ô × $20. Tiền khởi tạo $125.000, ticket 10. Mở đất miễn phí.

**Tất cả giá, nhịp thu, phần thưởng, rank và lịch mở đất trên là luật sandbox tạm**,
không phải số liệu kinh tế gốc đã khôi phục. Ticket có giá trị cao hơn tiền thường
thông qua nâng cổng và chỉ được thưởng khi lên rank, chưa có mua bằng tiền thật.

## Giao diện và asset

- Góc trái: PARK RANK, tiến độ flag đỏ, hạng/streak và phần trăm doanh thu.
- Dưới trái: build, map (chưa dùng), settings, nhiệm vụ (chưa dùng), khách.
  Dưới cùng hiển thị tiền/ticket. Settings có nhạc, save và debug; F1 vẫn bật debug.
- Shop: ảnh môi trường trên; thumbnail công trình nền trong suốt trên thẻ cam ở
  dải dưới. Dùng cặp ảnh gốc đã đối chiếu; Wave Rider/Log Flume dùng 0x8ce/0x8e0
  để thấy cả công trình. Mục chưa gán thumbnail dùng model gốc và khung camera vừa hình.
  Mô tả tiếng Anh hiện là mô tả sandbox, chưa khôi phục toàn bộ localized strings.
- Tay nhấn 0x108; mặt buồn/vui/quá tải 0x10c/0x10e/0x110; tiền/flag từ atlas gốc.

## Cổng và bản đồ

- Đã đo khoảng hở rào 669/670/671: nằm từ x≈-32 tới 0, tâm **x=-16**, không phải 0.
  Cổng 216–219 đặt ở tâm này; sửa chung cho ride/shop, các cỡ ô và preview.
- Bounds bỏ qua mesh locator. Công trình khác nhà phao được vừa vào ô, chừa lối
  trước cổng; xét cả chiều xoay 90/270 độ. Đây là cách bố trí desktop tạm, cần tiếp
  tục xác minh scale và theme từ mã gốc.
- Cổng công viên 633 vốn có đường chữ U: bỏ lớp đường nối chồng lên U; ẩn phần
  đường tĩnh nằm đúng footprint U. Mép vỉa hè tiếp giáp mép trước U. Ghép tường
  vàng 323 hai bên cổng. Đường dài từ x=-89 đến 271 ô, thêm cây/đèn dọc tuyến.
- Mây dùng texture 0x1a2 và model 631 gốc: che ô khóa và vùng ngoài terrain.
  Camera vẫn kéo qua được; nền ngoài bản đồ không còn đen. Vị trí/bố trí mây là mới.

## Animation và giới hạn

Nhà phao nảy theo đồng hồ riêng của ride; bốn animation đã khôi phục (425, 427,
515, 522) dùng đồng hồ này để tăng tốc theo nhấn. Ride chưa có liên kết animation
gốc chỉ có chuyển động lắc nhẹ tạm khi chơi; chưa thể coi là operating animation
gốc. Chỉ nhà phao hiện khách đang chơi, các ride khác còn cần passenger attachment.

## Save và kiểm chứng

Save **TPPLAY6** / `playtest-v6.txt` lưu tiền, ticket, flag, streak, phần thưởng chờ,
đồng hồ thu, trạng thái minigame và tốc độ animation. Tự nhập v5/v4/v3/v2 khi chưa
có v6, giữ nguyên file cũ. Với v5 giữ trạng thái ô; save cũ hơn dùng cách mở ô tạm.
Không dùng thời gian hệ thống để cộng lợi nhuận offline khi game đóng.

Test mô phỏng kiểm tra thu trùng, thiếu tiền, cả hai kiểu upgrade, minigame thành
công/nhấn quá nhanh, thưởng ticket khi lên rank, bán có khách, lưu/tải và migration.
Test native dùng click thật qua handler: mở chi tiết, thu, nhấn, nâng, bán và save.
Bằng chứng: `analysis/runtime_test/economy_test.json`, `ride-minigame.png`,
`reward-cash-flags.png`, `cloud-horizon.png`, `original-ui-rides.png`.

Code: `src/park_simulation.h` (luật/save), `src/park_game.h` (render/interaction),
`src/park_original_ui.h` (HUD/shop), `src/themepark_3d.cpp` (native regression).
