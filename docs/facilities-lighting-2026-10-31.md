# Staff, nhà chính và ánh sáng — 31/10/2026

## Cách sử dụng

- B mở Build, chọn STAFF → Staff House, rồi chọn ô 4×4 đã mở. Giá tạm $1.800, xuất hiện một nhân viên. Khi mở menu từ ô đã chọn, nút BUILD đặt nhà ngay vào ô đó; nếu ô còn khóa, nút UNLOCK mở ô trước.
- Nhân viên đi theo mạng đường, chỉ trong territory chứa nhà. Ưu tiên thùng rác đầy, đi đến đường gần thùng, dừng 3 giây để dọn và nhận 2 flag theo cơ chế dọn thủ công. Nếu không có việc thì tuần tra. Không tiêu thụ NPC khách hoặc ghế ride.
- Bấm nhà chính ở territory cổng để mở PARK OFFICE. Chọn CASTLE, CANDY hoặc PALACE miễn phí; LEFT/RIGHT/UPPER giá $2.500 mỗi phần, mua một lần. Nâng cấp căn nhà ban đầu tự chuyển sang kiểu Castle. Đổi kiểu giữ các phần đã mua. Theme đường/cỏ khu cổng vẫn Classic.
- Ô 16×16 chỉ có RIDES; kiểm tra mua ở simulation cũng chặn shop, không chỉ ẩn tab. Shop 16×16 đã có trong save cũ vẫn được giữ.

## Asset và đồ họa

- Nhà staff dùng assembly 29; các frame nhân viên gốc 11–22. Model 28, 30, 31 cũng đã kiểm tra nhưng chưa đưa thành lựa chọn staff house riêng.
- Nhà chính: Castle 252–258, Candy 259–265, Palace 266–272. Dùng vị trí cánh và tầng đã có trong model; nền 274–277 tương ứng không cánh/trái/phải/cả hai. Nhà ban đầu 273 giữ nguyên đến khi đổi kiểu hoặc nâng cấp.
- Cổng 633 dịch nửa ô theo X; giữ lại hàng path 142,5 trước đây bị ẩn nhầm. Hai nhánh cổng trùng với tâm đường gốc 90,5 và 92,5. Tường quanh cổng dịch theo cùng tâm mới.
- Đèn ban đêm tăng dần theo pha ngày/đêm: vùng sáng ấm trên nền quanh các ride/shop hoàn tất, cùng tối đa 7 point light gần tâm vùng camera đang xem. Khoảng suy giảm ánh sáng tính theo thang tọa độ renderer. Đây là giới hạn của renderer fixed-function hiện tại; công trình xa vẫn có quầng nền nhưng không phải mỗi công trình đều có đèn chiếu 3D đồng thời.
- Thêm bóng tiếp xúc mềm dưới ride và NPC, nhẹ vào ban ngày. Chưa có shadow map hoặc bóng đổ hình học đầy đủ. Giữ texture, model, góc camera và bảng màu hiện tại.

## Lưu game và kiểm chứng

Save TPPLAY10 giữ nhà staff, theme nhà chính và phần mở rộng; đọc được các bản cũ. Khi nạp save, nhân viên bắt đầu tuần tra lại từ lối vào nhà; vị trí giữa chặng không được lưu. Tiền/ride/khách hiện có không bị reset.

53 Python tests đạt. Simulation kiểm tra mua nhà staff, chiếm ô, tuần tra không vượt territory, dọn đúng thùng, thưởng flag, mua phần nhà một lần, lưu/đọc lại và chặn shop 16×16. Native kiểm tra menu staff, ba bộ nhà chính, tab ô lớn và dọn rác trên bản đồ thực. Ảnh kiểm chứng nằm trong `analysis/runtime_test/`: `staff-shop.png`, `staff-patrol.png`, `office-0.png` đến `office-2.png`, `office-menu.png`, `large-rides-only.png`, `night-local-lights.png`, `entrance-connected.png`.

## Hướng cải thiện tiếp

Chưa cần đổi engine để giữ phong cách này. Bước tiếp theo hợp lý là cache/batch hình học tĩnh và culling trước, rồi thêm shadow map độ phân giải vừa phải cho vùng camera, lọc bóng mềm và emissive riêng cho cửa sổ/bóng đèn. Cần đo lại trên GPU thực trước khi tăng số đèn hoặc thêm bloom; không nên phủ bloom mạnh lên texture màu sẵn của game.
