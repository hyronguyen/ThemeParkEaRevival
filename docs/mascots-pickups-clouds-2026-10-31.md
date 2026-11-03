# Mascot, vật phẩm rơi và khởi động — 31/10/2026

## Các thay đổi

- STAFF có đủ 5 nhà trong nhóm asset 28–32. Nhà 29 đã chuyển từ nhân viên xây dựng sang mascot gà. Các cặp đang dùng: Jungle 28 → nhân vật lực sĩ 45; Chicken 29 → gà 36; Shark 30 → cá mập 39; Bear 31 → gấu 42; Rhino 32 → tê giác 33. Khung chuyển động lấy từ dãy frame gốc cách nhau 14 model, gồm 12 frame. Mapping ngoài cặp gà được đối chiếu theo hình ảnh/cụm asset; chưa có bảng cấu hình máy chủ để xác nhận tên thương mại gốc.
- Mỗi nhà spawn mascot riêng, vẫn tuần tra và hỗ trợ dọn thùng rác trong territory. Nhà staff cũ trong TPPLAY10 tự nhận loại Chicken khi tải lại.
- Bổ sung bóng tiếp xúc mềm cho tượng theme, nền nhà chính và nhà staff; vẫn là bóng nền mềm, không phải shadow map.
- Bấm biểu tượng lợi nhuận sẽ tung ra tối đa 5 item tiền và 3 item cờ. Tổng giá trị được chia chính xác giữa các item. Bấm từng item mới nhận tiền/XP; không còn cộng ngay rồi hiện dòng chữ. Cuối minigame và dọn rác thủ công cũng tạo item. Staff tự dọn vẫn thưởng flag tự động.
- Item không tự biến mất. Save TPPLAY11 lưu cả item chưa nhặt và loại mascot; đọc được TPPLAY10 cùng các save cũ. Thoát game bình thường vẫn tự lưu. Nhân viên bắt đầu lại từ nhà khi nạp, như bản trước.
- Xác suất khách vào ride tăng 50% → 90%. Có khách đầu tiên là bắt đầu lượt 20 giây, không chờ đủ khách hoặc 1 phút. Khách đến sau điền ghế trống, giữ nguyên ghế người đang chơi và tham gia phần thời gian còn lại của lượt. Minigame vẫn không nhận thêm khách tự nhiên.
- Mây khu khóa giảm 64 → 16 cụm/khu, lớn và lệch hàng nhẹ, nâng lên cao, không chịu ánh sáng đêm và vẽ sau cảnh 3D. Cầu giữa khu mở và khu khóa có barricade/biển cấm từ asset 10; biến mất theo trạng thái mở khu.
- Nhúng icon gốc Android vào executable Windows (nhiều kích thước ICO). Màn hình khởi động hiển thị tiến độ theo các bước tải asset và khôi phục save, rồi nút ENTER PARK/Enter. Mô phỏng không chạy phía sau menu.

## Kiểm tra

53 Python tests đạt. Simulation kiểm tra admission 90%, chạy ngay khi có khách, giữ ghế, bảo toàn tổng tiền/XP của item, không nhận trùng, lưu/đọc item chưa nhặt, staff và chức năng trước đó. Native self-test đạt: 40 khách, 6 ride, 182 lượt hoàn tất sau 348,95 giây mô phỏng; click nhận item qua hit region; icon resource có thật; Enter đóng menu và timer không chạy sau menu.

Đã xem ảnh của cả 5 cặp nhà/mascot (`analysis/runtime_test/mascot-house-0.png` đến `mascot-house-4.png`), item rơi (`reward-cash-flags.png`), mây/biển chặn (`territory-boundary.png`) và menu (`startup-menu.png`). Các file này là ảnh test, không thay đổi save người chơi.

Đã cập nhật `build/ThemeParkGame.exe` và bản sao `build/ThemeParkGame-update.exe`: 1.104.896 byte, SHA-256 `304bc495cc867e8b4506512d372705ec3248ac1fd4a33193c63cd8bd83b92f1b`. Executable native AMD64; kiểm tra icon nhúng và vòng kiểm tra cuối đều đạt.
