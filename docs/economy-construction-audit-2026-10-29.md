# Giá, XP và giàn giáo — 29/10/2026

## Dữ liệu kinh tế xác minh được

Khôi phục 65 số nguyên từ `Global::_Construct::XPTable_values` trong IPA iOS 1.8.3, offset 5020912. Hàm khởi tạo tại 0x15f376 đặt số phần tử 0x41 và sao chép 0x104 byte. `Control::incrementProgress` tại 0x8619c đọc `XPTable[level-1]`, trừ lượng XP của cấp cũ khi lên cấp.

Các mức đầu: 400, 400, 600, 500, 200, 520, 850, 1320, 2310, 3600. Đây là bảng mặc định đóng gói trong client; parser `ConfigKeyValue` có thể nhận bảng XPTable thay thế từ dịch vụ. Không khẳng định đây là cân bằng server cuối cùng hay cùng phiên bản với video tham chiếu.

Đã áp dụng bảng vào logic cấp độ và HUD. Save giữ nguyên tổng flag, tiền và ticket; cấp hiển thị được tính lại theo bảng mới, nên có thể khác cấp cũ. Không tự phát ticket hồi tố khi tải save. Khi đang chơi, vượt nhiều mốc thưởng đủ số cấp; dọn rác cũng đi qua cùng hàm thưởng, sửa lỗi thiếu ticket khi lên cấp bằng dọn rác. Thưởng một ticket/cấp vẫn là luật PC. Giới hạn hiển thị 65 cấp để không đọc vượt bảng; không suy diễn thêm mốc XP.

Giá mua từng unit **chưa khôi phục được bảng số liệu**. Android parser `Ldk/progressivemedia/skeleton/h/f;::a`, code offset 3294944, có `price`, `priceSuperTickets`, `sellPrice`, `upgradeCost`, `buildHeight`, `buildTimeInSeconds`, `earningsPerCollect`, `parkRankPerCollect`. Parser LoadData đọc Entity và ConfigKeyValue từ phản hồi dịch vụ. Kiểm tra các member APK/IPA ngoài DEX/binary không tìm thấy bảng XML chứa những trường này. Tài nguyên đóng gói còn các định dạng nhị phân; kết quả này không chứng minh tuyệt đối rằng mọi bảng đều vắng mặt.

Không thay giá tạm bằng giá đoán. Giá PC hiện vẫn diện tích × $20, nâng cổng $100 × (cấp + 1), bán nửa giá xây + $50/cấp. Nguồn đầy đủ: `analysis/economy_defaults.json`, `analysis/economy-package-search.json`, `analysis/ios/economy-defaults-audit.txt`, `analysis/ios/xp-increment-audit.txt`. Generator: `tools/recover_economy_defaults.py`; bảng được biên dịch trong `src/original_economy.h`.

## Sửa xây dựng

Lỗi trước: đặt roof và side cùng z=0 rồi co chiều cao riêng mái. Vì vậy side nhô qua mái thành hộp hở. `Construction::createScaffold` khởi tạo một roof và các side cách nhau 32 đơn vị; `getRoofPart/getSidePart/getDirtPart` chọn bộ theo footprint.

Bản sửa ghép side tại 0 và 32, roof nguyên hình tại 64, dirt tại mặt đất. Cỡ 4×4 dùng 638–640, 6×6 dùng 641–643, 16×16 dùng 644–646. Ba tầng tạm theo ảnh tham chiếu vì chưa có `buildHeight` riêng của từng entity. Không co mái theo thời gian. Thợ ở từng tầng luân phiên đi bộ/đóng búa bằng frame gốc 16–27; phối hợp thời gian là bản PC, chưa phải khôi phục toàn bộ state machine gốc. Xây vẫn hoàn tất tự động sau 20 giây theo yêu cầu trước.

## Kiểm chứng

53 Python tests đạt; kiểm tra trực tiếp bảng biên dịch với byte IPA, chiều cao side 32. C++ simulation kiểm tra tất cả ranh giới XP, nhiều cấp cùng lúc, giới hạn cấp, ticket từ thùng rác, save/load và các hành vi hiện có. Native self-test đạt: 40 khách, 6 công trình, 120 lượt trong 358,55 giây mô phỏng. Đã xem ảnh xây 4×4 và 6×6; native xuất cả 16×16.

Game cũ đang mở nên build ghi vào `build/ThemeParkGame-update.exe`, không thay exe đang dùng. Kích thước 1002496 byte; SHA-256 `c5c80aca73bf854979f7b41a28ef0f92297feb0d896a12779a221a8f7cc37853`.

## Ưu tiên cải thiện tiếp

1. Cache mesh tĩnh cho thùng rác, hàng rào, giàn giáo; renderer hiện gửi vertex của nhiều vật qua immediate mode mỗi frame. Đo riêng simulation/render/GPU trước và sau, không kết luận FPS từ ảnh.
2. Nội suy vị trí NPC và thời gian animation giữa các bước simulation 20 Hz để chuyển động mượt hơn mà không đổi luật chơi.
3. Thêm chỉ báo phân biệt thông số đã khôi phục và thông số PC trong công cụ kiểm tra; ưu tiên tìm phản hồi Entity/ConfigKeyValue lịch sử để hoàn thiện giá, thu nhập, thời gian và buildHeight theo từng unit.

Các mục này là đề xuất tiếp theo, chưa được triển khai trong bản này. Phép đo native hiện tại median 58,78 ms/p95 62,80 ms không tính presentation; game khác vẫn mở nên không dùng làm so sánh hiệu năng trước/sau đáng tin cậy.
