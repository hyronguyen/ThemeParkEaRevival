# Sửa tương tác theo ảnh người dùng — 27/10/2026

- SFX: click 137, chọn công trình 135, xây 93, thu thưởng/nâng cấp 131, bán 84, mở khu 87. Truy nguyên ở sfx-original-mapping.md. Tắt mapping tự đoán cho tap/khách lên/xuống. Âm lượng 30%, debounce và không chồng cùng một sample khi đang phát. Nâng cấp dùng âm hoàn tất; chưa phục hồi mọi điều kiện/âm lượng gốc.
- Camera: chuột trái kéo để pan, thả khi chưa kéo mới chọn; chuột phải vẫn pan. X/Escape phục hồi snapshot trước khi vào chi tiết ngay lập tức (vị trí, yaw, pitch, zoom, pan, radius). Không chọn ride khác trong detail. Bỏ khoảng đệm rời rạc giữa territory đã mở, giữ giới hạn vùng đã mở.
- Công trình: giữ origin tác giả; bỏ recenter theo bounds của hình trang trí, bỏ thu nhỏ chừa 20/32 đơn vị và đẩy lùi 6 đơn vị. Lối vàng Fries đi tới y=-64 đúng cạnh ô 4×4 và khe rào. Mô hình quá footprint vẫn có giới hạn scale. Các ride xoay tùy ý/footprint đặc biệt vẫn cần đối chiếu artwork riêng; không coi tất cả là đã pixel-match.
- Cổng công viên: nối thêm hai đoạn tường vàng từ mép tường x=±96 tới chân trụ x=±75.2; mesh U-path gốc chạm mép vỉa hè đường. Ảnh entrance-connected.png xác nhận không còn hở hai bên.
- NPC: bảng fittedPassengerPose chọn pose riêng cho coaster, trượt nước, thuyền, carousel, tháp, ghế treo, v.v.; giữ ma trận orientation của locator gốc, không ép tất cả khách cùng hướng camera. Pose được đối chiếu hình asset; trừ car/kart đã có bằng chứng binary, đây chưa phải toàn bộ liên kết AnimSet gốc được khôi phục. Vẫn cần nghiệm thu các ride phức tạp từng chiếc. Không đổi/tự đoán lại chuyển động đường ray.
- Minigame: tap bắt đầu lấp đầy chỗ ngay, ưu tiên khách đang ngoài ride; không giật khách khỏi ride khác. Giữ lượt 30 giây, không chia thành đợt ngắn. Mức vui ổn định vẫn 30–80%, tap tăng tốc. Hết lượt dừng và tự cộng tiền/flag một lần; mức thưởng cơ bản 30/2, tốt 80/5. Không tự khởi động lượt mới khi spam click; đóng/mở lại để chơi tiếp. Đóng giữa lượt giải phóng khách, không nhận thưởng hoàn tất. Ride không có seat locator không tự tạo tư thế khách giả. Giới hạn khách trong save tăng lên 256 để đủ ghế khi cần; spawn bus thường vẫn giới hạn 96.

## Kiểm tra

Python kiểm tra PCM với nguồn gốc; simulation kiểm tra đủ ghế, còn khách trước mốc 30s, dừng sau mốc, không khởi động lại và hủy lượt không kẹt NPC. Native kiểm tra pan chuột trái, snapshot camera có pan, khóa chọn ride khác, SFX và cộng thưởng khi hết 30s. Ảnh trong analysis/runtime_test: motion-550-b.png, entrance-connected.png, motion-465-b.png, motion-522-b.png, session-30s-finished.png.

Không sửa IPA/APK gốc. Các WAV cũ có thể còn trong thư mục export nhưng game không gọi các event đã bỏ.

Kết quả cuối: 49/49 Python tests đạt; simulation và native self-test đạt, bao gồm auto-credit 30s và kéo chuột trái. Build SHA-256: ec119ca2e71e7e271ee829844fd28dce58e24c390ecb3a761ba05e9a418243af. Native SFX test phát âm lượng 0 để xác minh API, chưa phải đánh giá chất âm bằng tai.
