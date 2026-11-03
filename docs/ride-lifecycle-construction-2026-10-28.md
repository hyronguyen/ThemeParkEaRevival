# Camera, theme, lượt chơi và xây dựng — 28/10/2026

Tài liệu này thay thế thông số theme và lượt chơi trong các báo cáo trước.

- Territory 17 ngay cổng cố định Classic/văn phòng, kể cả khi đọc save cũ.
- Nền xác minh từ `ThemeSlot::createDecorations`; cổng và hàng rào công trình từ `GameMap::_Construct::*Walls_values`, thay bảng hàng rào ô trống đã dùng nhầm. Sky/Waterworld thay nhãn Fairytale/Volcano trước đây.
- Camera detail trực giao, hướng chéo trước, nhắm tâm công trình. Pitch nội bộ 35° tính từ trên tương ứng 55° xuống từ mặt phẳng ngang. Khóa kéo, zoom và phím xoay; thoát trả view trước.
- Căn chữ rank, giá, Build/Upgrade/Sell và vẽ thanh minigame bằng UI native, giữ font/icon gốc.
- Minigame 20 giây, độ vui giảm 14 đơn vị/giây; không hiện phần trăm hay đếm ngược. Nhân vật thêm để đầy ghế chỉ hiển thị trong ride, không tạo NPC ngoài đường khi kết thúc/thoát.
- Lượt tự nhiên: giữ số ghế đã nhận; đủ khách chạy 20 giây, chờ tối đa 60 giây nếu chưa đủ. Khách rời khi hết lượt.
- Mua ride/shop mất 20 giây xây, dùng giàn giáo gốc 638–646 cho cỡ 4×4, 6×6, 16×16 và thợ xây chuyển động. Chưa hoàn thành không nhận khách/sinh thu nhập. Thời gian và tiến độ là luật PC theo yêu cầu, không khẳng định lịch gốc.
- 173 vị trí thùng rác lấy từ `themedDecData` loại 4; chỉ hoạt động trong territory mở. Model 331/332, Waterworld 333/334. Đầy sau 60–180 giây; CLEAN nhận 2 flag và không nhận lặp khi đã sạch.
- Save TPPLAY9 lưu tiến độ xây, lượt chơi, ghế, thùng rác; đọc save 2–8.

Nguồn audit: `analysis/ios/theme_construction_audit.txt`, `theme_decoration_audit.txt`. Xuất dữ liệu bằng `tools/prepare_territory_themes.py`. Không chỉnh IPA/APK hoặc asset nguồn.

51 Python tests, simulation C++ và native self-test đạt. Native kiểm tra khóa camera, 12 theme, xây ba kích thước, dọn rác và save/load. Báo cáo `analysis/runtime_test/session-construction-trash-test.json`. Kịch bản tổng: 40 khách, 6 công trình, 120 lượt trong 358,55 giây mô phỏng. Đã xem ảnh detail, shop, giàn giáo 16×16 và CLEAN; đây là kiểm tra mẫu, chưa bao phủ mọi tổ hợp ride/theme.

EXE: `build/ThemeParkGame.exe`, 999936 byte; SHA-256 `cc262d3100d3f47cb78aebbb91e5d3b21792b3a6a95dd97aaa85d550430cf875`.
