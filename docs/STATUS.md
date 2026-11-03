# Trạng thái hiện tại

## Mascot, item rơi, khu khóa và khởi động — 31/10/2026

5 nhà staff/mascot thay nhân viên xây dựng (nhà 29 → gà), bóng nền cho tượng/nhà chính/staff; lợi nhuận tung tiền/cờ để click nhận, lưu item chưa nhặt bằng TPPLAY11. Admission 90%, ride chạy ngay và nhận thêm người vào ghế trống. Mây 16 cụm/khu vẽ trên cùng, biển chặn cầu khu khóa. Icon EXE và menu loading/ENTER PARK. [Chi tiết và kiểm chứng](mascots-pickups-clouds-2026-10-31.md).

## Staff, nhà chính, đèn đêm — 31/10/2026

Thêm STAFF HOUSE 4×4, cleaner tuần tra/dọn rác trong territory; PARK OFFICE có ba kiểu và mở cánh trái/phải/tầng trên. Save TPPLAY10 đọc save cũ. Ô 16×16 chỉ cho xây ride. Sửa đường nối cổng lệch nửa ô, thêm point light/quầng sáng đêm và bóng tiếp xúc mềm. 53 Python tests, simulation và native kiểm tra cuối đạt; đã cập nhật cả `build/ThemeParkGame.exe` và `build/ThemeParkGame-update.exe`. [Cách dùng, asset và giới hạn](facilities-lighting-2026-10-31.md).

## Bản mới nhất — 31/10/2026

Tách idle/vận hành, bổ sung clip gốc suy luận và hiệu ứng tia nước; 52 ride đạt kiểm tra chuyển động runtime. Giá tự cân bằng theo yêu cầu: Castle 2.600/3s, Teacups 6.300/10s, Ferris Wheel 6.800/15s. Tiếng khách ở giây 16,5 của minigame; thử ánh sáng ngày/đêm chu kỳ 6 phút, có nút tắt. Còn thiếu khôi phục đầy đủ 414, 415, 422 và 432. 53 Python tests, simulation và native self-test đạt. Bản chính `build/ThemeParkGame.exe` đã cập nhật. [Bảng giá, từng ride và giới hạn kiểm chứng](motion-balance-light-2026-10-31.md). Các mục dưới đây là lịch sử, không thay thế trạng thái mới nhất.

## Bản mới nhất — 29/10/2026

Áp dụng XPTable mặc định gốc iOS thay mốc cố định 400; giá unit còn thiếu dữ liệu, giữ giá tạm. Sửa giàn giáo thành ba tầng 32 đơn vị, mái kín nguyên hình và thợ đi/đóng búa từng tầng. Dọn rác lên cấp cũng thưởng ticket. 53 Python tests, simulation và native self-test đạt. Game đang mở nên bản mới là `build/ThemeParkGame-update.exe`. [Báo cáo dữ liệu, kiểm chứng và đề xuất](economy-construction-audit-2026-10-29.md).

## Bản mới nhất — 28/10/2026

Cố định theme tile cổng; sửa mapping nền/cổng/rào theo binary gốc; khóa camera detail ở góc chéo 55°; căn UI; minigame và lượt tự nhiên 20 giây, ghế cố định, chờ tối đa 60 giây. Thêm xây dựng 20 giây và 173 vị trí thùng rác, dọn nhận flag. Save TPPLAY9 tương thích save cũ.

51 Python tests, simulation C++ và native self-test đạt. EXE 999936 byte, SHA-256 `cc262d3100d3f47cb78aebbb91e5d3b21792b3a6a95dd97aaa85d550430cf875`. [Chi tiết và giới hạn kiểm chứng](ride-lifecycle-construction-2026-10-28.md). Các mốc phía dưới là lịch sử.

Cập nhật: **27/10/2026**. Điểm bắt đầu cho phiên phát triển tiếp theo.

Bản mới nhất: [animation, camera cổng và SFX](motion-camera-sfx-2026-10-27.md).
Tám chuyển động cơ khí tạm; camera chi tiết chuyển mượt về mặt cổng; tám SFX gốc.
Mây chỉ phủ khu đất lớn chưa mở và biên; ô xây khóa trong khu mở dùng đá.
UI panel được vẽ lại, giữ font/icon gốc; chi tiết ride ẩn HUD chính.
Executable chính đã cập nhật: `build/ThemeParkGame.exe`.
**Chưa khôi phục đầy đủ animation vận hành mọi ride**; xem giới hạn trong ghi chú mới.

## Bản chơi dùng được

- Chạy `build/ThemeParkGame.exe`; giữ thư mục `extracted/` ở gốc project.
- Windows x64 native, Win32/OpenGL, âm thanh WinMM; không cần emulator.
- UI dùng atlas/thumbnail/font Kabel gốc; B mở danh mục, F1 bật/tắt debug.
- 163 mục catalog giữ tương thích save; shop chỉ hiển thị 95 mục có artwork.
- Khách đến cổng có 50% cơ hội vào nếu còn chỗ, **không có hàng chờ ride**.
- Nhà phao hiện khách tức thì khi nhận, nhảy, rồi trở lại đường khi hết lượt;
  chỉ chạy khi chính ride có khách. Có nền mương nước/cầu gỗ, tường rào công trình
  và cổng 4 cấp. G nâng cổng bằng tiền, không còn quay vòng cấp.
- Bấm ride mở chi tiết, chơi minigame 7 giây, nâng bằng tiền/ticket hoặc bán.
- Tiền, ticket, flag/rank, streak cộng doanh thu, lợi nhuận định kỳ và hiệu ứng thu.
  Luật kinh tế là thông số sandbox tạm; xem tài liệu mới nhất ở trên.
- Nhà chính ở khu đầu tiên, 328 dòng trang trí gốc, đường/cổng/xe buýt đón trả khách;
  10 biến thể UV trang phục. O xem khu nhà chính và cổng.
- Ba bản nhạc MP3 gốc; điều khiển trong bảng debug.
- Save TPPLAY7 tại `%LOCALAPPDATA%/ThemeParkPreservation/playtest-v7.txt`.
  Tự nhập v6/v5/v4/v3/v2 khi chưa có v7, không ghi đè save cũ.

## Những phần chưa phải hành vi gốc được khôi phục

| Phần | Giới hạn hiện tại |
|---|---|
| Catalog | Tên/phân loại một số mục do nhận diện hình ảnh; chưa có catalog máy chủ đầy đủ |
| Ride | Mới có bốn liên kết idle animation được chứng minh; chưa đủ operating animation |
| Hành khách | 87 công trình có locator hiện khách; tư thế đa phần còn tạm, hai ride xe có model khách gốc |
| Nhà phao | Độ nảy và cách điều phối tư thế là thông số tạm |
| Xe buýt | Model gốc; lịch 40 giây, vị trí đường/trạm và điều phối khách là tạm |
| Shops | Có lợi nhuận định kỳ tạm; chưa có hàng hóa hoặc tồn kho |
| Đồ họa | Chưa đầy đủ vật liệu/effect/themed decorations; một số tâm đặt và footprint còn suy ra |
| UI | Artwork/font gốc; bố trí desktop và logic mới; mission/map là placeholder |
| Tiến trình | Có kinh tế/rank/streak tạm; chưa khôi phục luật gốc, nhân viên, bảo trì, nhiệm vụ |

Người dùng đã cho phép thông số tạm để có bản chơi được, nhưng yêu cầu giữ asset
gốc và ghi rõ phần tạm. Quy tắc 50% cơ hội vào, không xếp hàng là yêu cầu trực tiếp.

## Mốc kiểm chứng gần nhất

- 47 bài Python đạt; C++ simulation kiểm tra xác suất, sức chứa, không hàng chờ,
  bus arrival, nhập save cũ, lưu/tải xác định và từ chối save lỗi.
- Native: 160 mục có ô đặt đúng cỡ; lọc shop, mở khóa, camera và animation đạt;
  thao tác SHOPS/BUILD bằng chuột,
  đặt shop, thu tiền/flag, minigame, nâng bằng hai loại tiền, bán và save đạt.
  40 khách, 6 công trình, 205 lượt trong 447,3 giây mô phỏng.
- Workbench kiểm tra 730 model đạt. Đã xem ảnh thật sau khi sửa depth/alpha.
- EXE 857.600 byte, SHA-256 `7dc88006b48d02c97d1ef0d72a87de6ede1f20a639022222e6b726e920b52a70`.
  Xem `analysis/runtime_test/economy_test.json`.

Các JSON/PNG tại `analysis/runtime_test/` là bằng chứng hiện tại; chạy test sẽ ghi
đè chúng. Tài liệu cũ trong `analysis/` giữ lịch sử, không dùng tham số cũ để thay
thế trạng thái ở đây. Chi tiết: [bản mở rộng mới nhất](../analysis/expansion_ui_gameplay.md).

Tiếp theo đọc [ROADMAP.md](ROADMAP.md) và [DEVELOPMENT.md](DEVELOPMENT.md).

## Cập nhật camera và audit animation 27/10/2026

- Camera chi tiết lưu và trả lại view trước đó; chặn chọn ride khác khi đang xem.
- Thêm motion theo node cho 467, Ferris Wheel 476, 511 (511 vẫn ẩn vì thiếu artwork). Tổng 11 motion cơ khí tạm; chưa tuyên bố khôi phục toàn bộ operating clips gốc.
- [Danh sách từng ride](ride-animation-coverage-2026-10-27.md), gồm 57 ride hiện trong shop.
- [Đánh giá hiệu năng và engine](performance-and-engine-2026-10-27.md).

Kiểm tra bản build cuối: 49 Python tests đạt; simulation C++ và native self-test đạt. Native kiểm tra click vào tâm ride khác không đổi selection, đóng/X và Escape giữa chuyển cảnh trả lại camera. Báo cáo `analysis/runtime_test/detail_return_test.json`, `motion_camera_sfx_test.json`, `render_timing.json`. Bản chạy `build/ThemeParkGame.exe`, SHA-256 `ac897e4c9744a71592818e7ba3ba6682ee5aedd43183dba438c149721cef376e`.

## Âm thanh, camera, lối vào và session 30 giây

Xem [ghi chú bản sửa](interaction-polish-2026-10-27.md). Bản này thay thế lượt minigame 7 giây và mapping SFX tạm trước đó.

## Animation 11 ride yêu cầu — 28/10/2026

Đã bổ sung operating clip cho 460, 425 (Water Slides), 461, 520, 418, 522 (Log Flume), 420, 427 (Skull River), 521, 452, 453. Clip gốc; binding suy luận theo hierarchy và kiểm tra native, chưa phải mapping server xác minh. 452/453 giữ khán giả tại chỗ, animate diễn viên/đạo cụ.

[Báo cáo từng ride](requested-rides-animation-2026-10-28.md). 49 Python tests, simulation và native self-test đạt; kiểm tra 24 mẫu toàn chu kỳ, 33 ảnh và dừng khi không có khách. Bản mới: `build/ThemeParkGame.exe`, 957440 byte, SHA-256 `6a0c123cae208c0d3a5299d436114740af6e3704d8bdee4b07c02bb6fc778fc1`.

## Theme từng tile, nước, NPC và biên park — 28/10/2026

Đã thêm 12 lựa chọn theme qua 20 ô statue riêng, đổi nền/path/hàng rào theo territory và lưu TPPLAY8 (đọc save cũ 2–7). Sửa alpha Water Slides, hai làn NPC và khoảng cách cùng hướng, điểm xuất hiện/rời ride không trùng; nối lại biên park ngoài path và cắt đoạn kênh thừa.

[Chi tiết và nguồn dữ liệu](territory-themes-and-path-polish-2026-10-28.md). 51 Python tests, simulation và native self-test đều đạt; native kiểm tra đủ 12 theme và lưu/tải. EXE `build/ThemeParkGame.exe`, 986624 byte; SHA-256 `8fc38ce5187713c00b09d837d063d173f75b5e40591ec2cb8e222d2e8745c95f`.
