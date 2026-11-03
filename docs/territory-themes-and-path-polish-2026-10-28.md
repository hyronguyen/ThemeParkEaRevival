# Theme từng tile, nước, NPC và tường bao — 28/10/2026

> Báo cáo lịch sử. Mapping theme, quyền đổi theme tile cổng và save bên dưới được thay thế bởi [bản cập nhật mới](ride-lifecycle-construction-2026-10-28.md).

## Cách sử dụng

Nhấn ô 5×5 dành riêng cho statue trong tile đã mở (tile khởi đầu dùng vị trí văn phòng 8×6). Chọn theme rồi nhấn APPLY. Tile chưa mở phải được mở trước. Ô này không nhận ride/shop. Classic trả về nền/path mặc định; ở tile khởi đầu khôi phục văn phòng. Chọn theme khác sẽ đặt statue tương ứng.

Có 12 lựa chọn: Classic, Carnival, Haunted, Alien, Fairytale, Pirate, Western, Snow, Underwater, Candy, Space, Volcano. Tên là nhãn mô tả hình asset trong bản PC, không khẳng định là tên thương mại gốc.

Theme chỉ đổi đúng territory được chọn: statue, mặt nền 36×36, path và hàng rào chung quanh ride/shop. Không đổi công trình/animation, không di chuyển ride/NPC, không đổi theme hàng xóm. Biên park và các kênh phân cách giữ thiết kế chung. Chi phí đổi theme hiện miễn phí theo luật playtest.

## Nguồn asset và giới hạn xác minh

- `tools/prepare_territory_themes.py` xuất `extracted/runtime/territory_themes.tsv` và `theme_plots.tsv`.
- 20 ô statue lấy từ bảng gốc `themeSlots`, không tự tạo ô đè lên ride/shop.
- Path: hàm gốc `ThemeSlot::getPathTexture` tại VA `0xc067c`; atlas từ `000116` trở đi, model 296–306.
- Mẫu hàng rào: đọc trực tiếp các bảng `RideSlot::_Construct::MODELS_SLOTS_THEMED_*_values` trong binary iOS. Với ride/shop, renderer chừa khe vào ở cùng tọa độ cổng; không ghép bừa các model ID liền kề.
- Statue dùng model gốc 335, 340, 345, 365, 355, 350, 360, 370, 375, 380, 383. Các cặp nền/statue và nhãn mô tả là lựa chọn dựa trên hình ảnh; chưa khôi phục toàn bộ cấu hình territory từ server.
- Giữ nguyên IPA/APK, texture và model gốc.

## Nước

Các dòng nước và splash của Water Slides 425 có alpha. Trước đây chúng ghi depth trước mặt hồ, khiến màu cỏ phía sau lọt vào kết quả pha màu và mặt hồ không vẽ được ở vị trí đó. Nay vẽ hồ/công trình trước, dòng nước/splash sau và tắt ghi depth cho lớp hiệu ứng. Giữ texture trắng gốc, không đổi màu cả hồ hoặc máng trượt.

## NPC

- Đi hai làn, lệch nhẹ khỏi tim đường; chuyển hướng bằng nội suy tại góc.
- Người cùng hướng giữ khoảng cách tối thiểu 0,48 ô trên đoạn đường chung; không đi xuyên người phía trước.
- Người ngược hướng vẫn đi qua nhau; giao cắt có thể che nhau theo phối cảnh.
- Nhịp bước chân chỉ tiến khi nhân vật thực sự di chuyển.
- Spawn, xuống bus và rời ride chọn điểm path gần nhất còn trống thay vì dồn tất cả về một tọa độ.
- Giữ nguyên xác suất vào ride 50%, không xếp hàng ở ride và minigame 30 giây.

## Tường bao

Dựng một biên trước thống nhất ở y=145,25, phía ngoài path. Cắt phần kênh/tường gốc thừa tại cùng biên. Tường nối với hai chân cổng bằng đoạn vuông góc; bỏ đường tường cũ cắt ngang path. Phủ đủ biên map 5×4 territory.

## Lưu và kiểm tra

Save mới `TPPLAY8` chứa 20 lựa chọn theme; vẫn đọc TPPLAY2–7, tự đặt theme mặc định. Vẫn dùng đường dẫn save cũ để công viên hiện có được giữ lại. Kiểm tra dữ liệu theme lỗi trước khi thay trạng thái đang chơi.

- Python kiểm tra asset theme tồn tại và toàn bộ ô statue không giao với ô ride/shop.
- C++ simulation kiểm tra giữ khoảng cách, đi ngược hướng, spawn không trùng, save theme và đọc save v7/v6 cũ.
- Native kiểm tra click ô statue, áp dụng 12 theme, không thay tile khác, lưu/tải; chụp 12 ảnh trong `analysis/runtime_test/territory-theme-*.png`.
- Ảnh menu: `analysis/runtime_test/territory-theme-menu.png`; biên park: `park-perimeter-joined.png`; nước: `audit-425-6.png`.

Kết quả cuối: 51 Python tests, simulation C++ và native self-test đạt. Bản game: 986624 byte, SHA-256 `8fc38ce5187713c00b09d837d063d173f75b5e40591ec2cb8e222d2e8745c95f`.
