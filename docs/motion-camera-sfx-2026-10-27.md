# Animation khi có khách, camera cổng và SFX — 27/10/2026

## Hành vi mới

- Đồng hồ chuyển động chỉ tiến khi ride đang mở và có NPC; thao tác tap vẫn tăng tốc.
- Bổ sung chuyển động cơ khí **tạm**, dựng trên node gốc: Pirate Ship 409,
  Fountain Ride 410, Rocket 430, tower 445, Teacups 449, Parasol Chairs 463,
  Carousel 465, swing chairs 471. Chỉ bộ phận chơi chuyển động; terrain/root đứng yên.
  NPC dùng cùng ma trận node với ghế. Một số ride dùng model ngồi 411 gốc làm tư thế tạm.
- Nhà phao và bốn liên kết animation đã xác minh vẫn được giữ.
- Mở chi tiết: camera dịch chuyển, zoom và xoay mượt trong 0,85 giây về cạnh cổng
  (yaw 0 vì cổng hiện cố định ở cạnh dưới của mọi ô). Cổng không đổi vị trí.
  Cuộn chuột, kéo phải, Q/E, thoát hoặc mở shop ngắt chuyển cảnh.
- Tám SFX: click, tap, build, collect, upgrade, sell, board, complete.
  Âm thanh gốc lấy từ PAC/wv, không tổng hợp tiếng mới. **Gán âm thanh vào sự kiện
  là tạm**, chưa chứng minh đúng lựa chọn của game gốc.
- Tám voice WaveOut độc lập với MP3; chống dồn tap dưới 90 ms, âm lượng SFX 45%.
  Settings có SFX ON/OFF riêng. Mất focus dừng SFX, thoát game giải phóng voice.

## Giới hạn

Chưa khôi phục toàn bộ operating animation của tất cả ride. Tám chuyển động trên
là phương án chơi được có ghi rõ tạm, không phải animation gốc đã giải mã.
Ride ngoài danh sách không tự được gán chuyển động đoán; vẫn cần phục hồi bảng
AttractionStruct và đối chiếu animation vận hành từng ride. Thông số góc/tốc độ tạm
nằm trong `gameMechanicalPose` ở `src/park_game.h` để thay bằng clip gốc sau này.

## Dữ liệu âm thanh

`tools/prepare_game_sfx.py` đọc PAC little-endian: header 20 byte, 141 sample records
16 byte, 134 playable records. Sound record có sample ID và tần số riêng (không
áp 44,1 kHz cho toàn bộ: tap dùng 22,05 kHz). PCM16 được bọc WAV, payload giữ
nguyên SHA-256. Xem `extracted/runtime/audio/sfx/provenance.json`.

Bằng chứng disassembly: `analysis/ios/audio_decode.txt`, ReadSamples 0x44e498,
LoadSample 0x44ea3c, Sample constructor 0x4476dc. Constructor tính chiều dài
sample bằng bytes/2 khi encoding=1. Audio gốc và PAC không bị sửa.

## Kiểm tra

- 49/49 Python, bao gồm byte-for-byte PCM/WAV và SHA-256.
- C++ simulation: ride trống không chạy đồng hồ; giữ các kiểm thử save, khách, kinh tế.
- Native: chuyển động/locator cùng đổi, root không đổi, ride trống trả về nghỉ;
  camera không nhảy ngay, kết thúc đúng yaw, cuộn chuột hủy; tám WAV phát qua WaveOut
  với volume 0 khi test. Đây là kiểm tra phát được, chưa phải đánh giá nghe bằng tai.
- Báo cáo `analysis/runtime_test/motion_camera_sfx_test.json`.
- Ảnh `ride-gate-front.png`, `motion-409-a.png`, `motion-409-b.png`,
  `motion-449-a.png`, `motion-449-b.png`, `settings-sfx.png` cùng thư mục.
