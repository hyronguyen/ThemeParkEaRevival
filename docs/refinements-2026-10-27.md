# Cổng, ô đất, camera và shop — 27/10/2026

## Thay đổi

- Cổng 216–219 dùng chung điểm đặt cách mép trước ô 16 đơn vị vào trong,
  theo tâm ô lối vào. Chân cổng nằm ở khoảng hở tường rào, thay vì trên đường.
  Áp dụng chung cho mọi công trình và cả preview.
- Nhà phao reset pose trước mỗi lần vẽ từng instance; chỉ biến dạng khi chính
  instance đó có khách. Khách được nhận tại cổng xuất hiện ngay tại locator trong
  nhà phao; hết lượt trở về node đường, không có đoạn đi vào/ra.
- Model đường 634 dài **256 đơn vị = 8 ô**, không phải 4 ô. Các đoạn nối tiếp nhau;
  nền cỏ đồng cao độ -5. Lối đi kết thúc ở vỉa hè, không kéo vào làn xe.
  Dùng đèn 636, biển trạm 635 và cây 629 gốc. Bus 295 có đầu hướng -X;
  giữ hướng model, chạy X giảm từ phải sang trái, dừng để đón/trả khách.
- Ô khóa hiện đá 647/648/649. Ô mở trống hiện rào trắng 650/652/653 theo cỡ
  4/6/16. Bấm ô khóa → UNLOCK; sau đó chọn RIDES/SHOPS đúng kích thước.
  Cả UI và simulation từ chối đặt khác kích thước.
- Shop dùng artwork trên, dải item bên dưới và panel cam từ atlas gốc.
- Cuộn chuột nội suy zoom theo thời gian, giới hạn 1,2–7; pitch renderer 22–58 độ
  (0 là nhìn thẳng xuống). Zoom gần hạ góc nhìn; yaw và tọa độ vật thể giữ nguyên.
  Focus không còn tự đặt lại yaw. Workbench giữ điều khiển riêng.

## Save và những thông số tạm

Save mới **TPPLAY5**, `playtest-v5.txt`, lưu tập ô khóa. Tự nhập v4/v3/v2 nếu chưa
có v5, giữ nguyên file cũ và các công trình đã xây. Với bản mới hoặc save cũ chưa
có trạng thái ô đất, tạm mở khoảng một phần ba ô trống theo ID ổn định; các ô khác
cần UNLOCK miễn phí. Đây chưa phải progression/giá mở đất của bản gốc.

Footprint công trình vẫn suy từ bounds, làm tròn về ô vuông 4/6/16 của bản đồ.
160/163 mục có ô khớp; ba utility 1×1 chưa có ô tương ứng và không được nhét vào
ô lớn. Catalog B vẫn giữ đủ 163 mục để tra cứu; shop mở từ ô chỉ hiện mục khớp.
Cần khôi phục metadata máy chủ để chứng minh footprint từng mục.

Độ nảy, lịch xe 40 giây, vị trí cây/đèn mới và đường cong camera là thông số
desktop tạm theo ảnh tham chiếu. Asset đều gốc. Chỉ nhà phao có khách được vẽ
đang chơi; các ride khác vẫn cần phục hồi passenger attachment/operating animation.

## Kiểm chứng và điểm sửa

- `tests/park_simulation_test.cpp`: ô khóa, cỡ nhỏ/lớn bị từ chối, lưu ô khóa,
  migration, xác suất 50%, sức chứa, không hàng chờ và deterministic resume.
- `verifyGame` trong `src/themepark_3d.cpp`: chuột mở khóa/lọc menu, animation
  nhà phao rỗng/có khách, attachment tức thì, zoom nội suy và yaw không đổi.
- `analysis/runtime_test/refinement_test.json` và các ảnh `aligned-gate-*`,
  `camera-*-tilt`, `entrance-and-plot-states`, `plot-*-shop` là bằng chứng mới.
- Logic/render trong `src/park_game.h`; save và placement trong
  `src/park_simulation.h`; bố cục UI trong `src/park_original_ui.h`.
