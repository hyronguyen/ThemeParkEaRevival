# Khu đất, cao độ ride, UI và hành khách — 27/10/2026

## Đã thay đổi

- Tách quyền mở **20 khu đất lớn (5×4)** khỏi quyền mở từng ô xây.
  Mây phủ khu chưa mở; trong khu đã mở, ô chưa mở là đá, ô trống đã mở có rào trắng.
  Bấm khu bên cạnh để mở miễn phí (thông số sandbox tạm).
- Camera giới hạn điểm nhìn trong các khu đã mở, vẫn nhìn thấy mây ở rìa màn hình.
  Giữ zoom–tilt và yaw. Khu đầu tiên cho phép nhìn đến đường trước cổng.
- Tường bao chạy cả bốn cạnh bản đồ 180×144 tile, chừa lối cổng chính.
  Giữ model cổng có sẵn đường chữ U, đường lớn và cây/đèn/biển gốc.
- Đặt ride theo cao độ tác giả Z=0 thay vì nâng cả model lên bằng minZ.
  Phần mesh âm Z vốn chìm trong nền được giữ đúng chỗ; hành khách dùng cùng transform.
- Vẽ panel bo góc/gradient trực tiếp, giữ font Kabel, icon và nút minh họa gốc.
  Shop giữ artwork môi trường ở trên, cutout công trình lớn ở hàng dưới.
  Chỉ **95 mục có artwork** được hiện trong shop; catalog 163 mục vẫn giữ nguyên thứ tự để đọc save.
- Khi mở chi tiết ride, ẩn rank/streak/tiền/ticket/nút chính; chỉ còn thao tác ride và thu lợi nhuận của ride đang chọn.
- Gắn khách trực tiếp vào `vis_loc` theo hierarchy gốc trên **87 công trình có locator** (50 mục có artwork hiện trong shop).
  Sức chứa dùng số locator; khi hết lượt trở lại cổng tức thì. Không thêm hàng chờ.
  Mỗi instance dùng animation riêng trước khi vẽ khách để tránh lấy pose của instance khác.
- Khôi phục 8 model xe hành khách cho ride 521 và 503 từ resource AnimSet gốc.
  Công cụ tái tạo: `tools/export_passenger_poses.py`.
- Save **TPPLAY7**, `playtest-v7.txt`; nhập v6/v5/v4/v3/v2 khi chưa có v7.
  Save cũ mở khu đầu tiên và mọi khu đã có công trình; khách vượt sức chứa mới trở lại cổng.

## Giới hạn khôi phục animation (mục 7 chưa hoàn tất)

Không có căn cứ để gắn toàn bộ 203 animation group vào từng ride.
Bốn liên kết idle đã được chứng minh vẫn là 425→47, 427→49, 515→151, 522→159.
Nhà phao giữ cơ chế đàn hồi/khách nhảy có điều kiện đang chơi.
Các ride khác có locator hiện được khách, nhưng tư thế đứng là tạm, trừ hai ride xe nêu trên.
Ride không có locator vẫn ẩn khách trong lượt chơi. Không dịch chuyển hay xoay cả công trình
để giả làm animation vận hành: cơ chế lắc cả model tạm trước đây đã bỏ.

`Ride::playRidingAnim` tại 0xb2538 tìm `vis_loc`, gắn PersonBase vào node, đọc
mã animation khách từ AttractionStruct +0x10c. Hai nhánh đặc biệt 0x209 (=521)
và 0x1f7 (=503) dùng các mảng carAnims/kartAnims. AnimSet::init (0x67518)
đọc số phần tử và danh sách model big-endian; 8 resource đã xác minh SHA-256.
Danh sách nằm tại `analysis/passenger_poses.json`. Quét resource XML trong IPA
không tìm thấy bảng định nghĩa animation vận hành còn thiếu.

Việc cần làm tiếp: khôi phục bảng AttractionStruct/game-definition tương ứng,
xác minh từng group bằng hierarchy và ảnh chuyển động, rồi thay tư thế đứng bằng
AnimSet đúng. Không suy luận ánh xạ chỉ từ số node hoặc thứ tự model.

## Kiểm chứng và đường dẫn

- `scripts/test-playtest.ps1 -GameExecutable ThemeParkGame-update.exe` kiểm tra executable mới
  khi game cũ còn đang mở. Không đóng tiến trình game của người dùng.
- Kiểm thử simulation: mở khu kề nhau, chặn mở cách xa, save/resume, nhập save cũ,
  giữ khách, năng lực ride, kinh tế và minigame.
- Native: `analysis/runtime_test/territory_test.json`, `refinement_test.json`, `economy_test.json`.
- Ảnh UI: `original-ui-rides.png`, `original-ui-shops.png`, `ride-detail-clean.png`.
- Ảnh map: `territory-boundary.png`, `territory-clouds-and-rocks.png`, `skull-river-grounded.png`,
  `water-slides-corrected.png` trong `analysis/runtime_test/`.

Kết quả: 48/48 kiểm thử Python, simulation C++ và native UI đều đạt.
Bản chính `build/ThemeParkGame.exe` đã được cập nhật sau khi hết khóa ghi.
