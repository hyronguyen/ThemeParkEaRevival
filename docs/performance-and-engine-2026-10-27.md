# Hiệu năng và lựa chọn engine — 27/10/2026

## Kết luận

Ưu tiên tối ưu bản C++ hiện tại trước. Chuyển engine không tự khôi phục được liên kết animation, đường ray hay tư thế NPC còn thiếu. Nếu mục tiêu dài hạn là có editor dựng cảnh, UI linh hoạt và phát hành nhiều nền tảng, nên thử một lát cắt nhỏ bằng Godot trước khi quyết định chuyển toàn bộ.

## Bằng chứng trong dự án

- `src/park_game.h::gameTick`: mô phỏng bước 0,05 giây (20 Hz). Camera cập nhật mỗi frame nhưng đồng hồ animation và vị trí NPC lấy từ bước mô phỏng. Vì vậy có thể thấy chuyển động giật từng nấc ngay cả khi renderer đủ nhanh.
- `src/themepark_3d.cpp::drawParts`: gửi từng vertex qua `glBegin/glVertex3f`; cảnh tĩnh có display list, các ride và nhiều vật trang trí vẫn đi qua đường vẽ trực tiếp. Không nên kết luận C++ chậm: đây là cách tổ chức renderer hiện tại.
- Vòng lặp dùng WM_TIMER 16 ms, không phải cam kết trình bày đúng 60 FPS.
- Đo bản build cuối: 30 frame sau 5 frame làm nóng, `render(false)` + `glFinish`, cùng cảnh native self-test 1264×800: median 13,16 ms, p95 14,04 ms. Không bao gồm SwapBuffers/presentation, không phải FPS thực khi chơi và chưa phải stress test toàn bản đồ. Kết quả lần chạy cuối nằm ở `analysis/runtime_test/render_timing.json`.

## Thứ tự tối ưu đề xuất

1. Nội suy vị trí NPC và thời gian hoạt ảnh giữa hai tick. Giữ logic/save ở bước cố định; reset nội suy khi khách lên/xuống ride để không tái xuất hiện đoạn đi vào cổng. Đồng bộ thời gian render của ghế và NPC.
2. Culling theo cụm territory và vùng camera. Không vẽ công trình nằm hoàn toàn ngoài màn hình, không cập nhật pose ride ẩn/trống khi không cần.
3. Đưa vertex/index tĩnh lên GPU, dùng ma trận node cho bộ phận động; gom cây, hàng rào, đèn cùng mesh/material thành batch hoặc instance. Đo trước/sau từng bước.
4. Tách đo simulation, pose, submit, GPU và presentation. Kiểm tra toàn cảnh và chi tiết với 100/300/1000 NPC; mục tiêu 60 FPS là tổng frame dưới 16,67 ms, không chỉ thời gian submit.
5. Chỉ sau đó cân nhắc thay renderer/engine. Không tăng tần số mô phỏng lên rất cao để che vấn đề nội suy.

## Nếu thử Godot

Giữ các công cụ giải mã và schema save độc lập. Lát cắt thử gồm Bouncy Castle, Ferris Wheel, một ride nước, NPC lên/xuống tức thì, camera chi tiết/trở về và một panel shop. So ảnh, kiểm tra UV/alpha, pivot, hierarchy, tọa độ, NPC đi cùng node, save và âm thanh trước khi chuyển thêm.

Godot có nội suy giữa các tick, phù hợp vấn đề 20 Hz hiện tại: [tài liệu chính thức về interpolation](https://docs.godotengine.org/en/stable/tutorials/physics/interpolation/physics_interpolation_introduction.html). Với vật thể lặp lại, [MultiMesh](https://docs.godotengine.org/en/stable/tutorials/performance/using_multimesh.html) hỗ trợ instancing, nhưng không cull từng instance riêng lẻ; cần chia theo khu vực. Đây là khả năng hữu ích để thử, không phải bảo đảm tăng FPS khi chưa benchmark cùng asset/camera.

Không đề xuất port toàn bộ ngay hoặc bỏ bản native đang chạy. Chi phí lớn nhất của việc chuyển là kiểm chứng importer, vật liệu và animation chứ không phải chỉ viết lại vòng lặp render.
