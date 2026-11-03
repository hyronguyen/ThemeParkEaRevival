# Các hạng mục phát triển tiếp

Danh sách đề xuất, chưa phải các tính năng đã hoàn thành. Ưu tiên khôi phục có
bằng chứng từ IPA/mã máy/video; nếu dùng thông số tạm phải ghi rõ.

Đã xử lý ngày 27/10: cổng sát rào, nhà phao chỉ chạy có khách, lên/xuống tức thì,
ô khóa/mở và shop đúng cỡ, đường/bus, camera zoom–tilt. Xem [ghi chú](refinements-2026-10-27.md).

Lượt 2 đã thêm minigame 7 giây, nâng/bán ride, tiền/ticket/flag, streak và thu lợi
nhuận định kỳ; đây là cơ chế sandbox tạm. Xem [luật và giới hạn](economy-and-ride-details-2026-10-27.md).

## P1 — Hoàn thiện một ride làm mẫu

- [ ] Chọn Water Slides hoặc Log Flume, xác minh operating animation và ghế/boat.
- [ ] Khách hiện đúng khi lên/chơi/xuống; không teleport giữa ghế khi người khác rời.
- [ ] Đồng bộ admission, animation, đóng/mở và save khi đang chơi.
- [ ] Khôi phục animation gốc của nhà phao, thay biến dạng tạm hiện tại.
- [ ] Kiểm tra cổng, cầu vào và đường đi khi xoay công trình.

Điểm sửa: `src/park_game.h`, `src/park_simulation.h`, `tools/extract_animations.py`.
Đạt khi có liên kết model/group được chứng minh, video/ảnh đối chiếu và test vòng
đời khách + save giữa lượt. Không suy liên kết chỉ vì hai asset có cùng số node.

## P1 — Chuẩn hóa catalog và bản đồ

- [ ] Xác minh tên, nhóm RIDES/SHOPS, thumbnail, theme và footprint cho từng mục.
- [ ] Tách rõ ride, shop và utility; không coi danh sách 163 mục là catalog gốc hoàn chỉnh.
- [ ] Đọc hết `themedDecData`; xác minh kích thước/tâm đặt của `decorations`.
- [ ] Khôi phục cổng/rào theo theme của từng công trình, thay mặc định Funland.
- [ ] Xác minh vị trí trạm/đường trước công viên và animation xe buýt gốc.

Điểm sửa: `tools/prepare_expansion.py`, `tools/export_park_scene.py`, `src/park_game.h`.
Đạt khi mỗi liên kết có nguồn, ảnh không chồng sai ô và giữ tương thích catalog/save.

## P2 — UI và thao tác chơi

- [ ] Tiếp tục đối chiếu màn thông tin/settings/nâng cấp đã dựng với logic gốc.
- [ ] Bổ sung trạng thái đang chọn, quay lại, mô tả và tìm kiếm/phân trang dễ dùng.
- [ ] Thêm chọn/xóa/di chuyển công trình với kiểm tra lối vào và hành khách đang chơi.
- [ ] Kiểm tra cửa sổ nhỏ, DPI 125–200%, phông chữ và vùng bấm.
- [ ] Giữ debug riêng, tránh đưa thông tin kỹ thuật vào giao diện chơi chính.

Điểm sửa: `src/park_original_ui.h`, `src/park_game.h`.
Đạt khi dùng artwork gốc, không cắt/chồng chữ, và có test chuột cho các luồng thay đổi.

## P2 — Đồ họa và âm thanh

- [ ] Vật liệu nước, alpha ordering, hiệu ứng và UV animation theo dữ liệu gốc.
- [ ] Thêm các appearance/body/animation set của khách ngoài 10 UV hiện tại.
- [ ] Giải mã PAC/raw audio để phục hồi SFX ride/khách/bus.
- [ ] Giảm số lần tải/vẽ và đo FPS ở công viên đông; chỉ tối ưu sau khi có số đo.

Điểm sửa: `src/themepark_3d.cpp`, `src/park_audio.h`, các exporter trong `tools/`.
Giữ test chống tái xuất hiện mesh `vis_loc`, lớp tô ô che nền và lỗi mất alpha blend.

## P3 — Gameplay và phân phối

- [ ] Khôi phục kinh tế shop/ride gốc thay luật tạm; nhu cầu khách, nhân viên/bảo trì.
- [ ] Rank, quest, mở vùng, nâng cấp và lưu tiến trình tương ứng.
- [ ] Version/save migration khi đổi dữ liệu; kiểm thử save cũ và file hỏng.
- [ ] Launcher/installer và kiểm tra máy Windows khác; quy trình trích xuất asset cục bộ.

Không giả lập máy chủ báo thành công hoặc đưa archive/asset độc quyền vào bản phát
hành công khai. Mỗi hạng mục nên kết thúc bằng thay đổi nhỏ có bằng chứng, test phù
hợp và cập nhật [STATUS.md](STATUS.md).

## Cập nhật 27/10: khu đất và hành khách

- [x] Tách mở khu lớn / mở ô xây; save v7 và giới hạn camera.
- [x] Shop lọc artwork, panel mới và ẩn HUD khi xem ride.
- [x] Gắn NPC vào locator cho 87 công trình, khôi phục model xe khách 521/503.
- [ ] Khôi phục bảng animation vận hành và tư thế khách cho toàn bộ ride; xem `territories-ui-passengers-2026-10-27.md`.

- [x] Camera chi tiết chuyển mượt về mặt cổng, điều khiển thủ công hủy chuyển cảnh.
- [x] Bộ tám SFX PCM gốc, bật/tắt riêng; ánh xạ sự kiện tạm.
- [x] Chuyển động cơ khí tạm cho tám ride có cấu trúc đã kiểm tra.
