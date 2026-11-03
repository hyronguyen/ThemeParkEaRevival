# Liên kết SFX tìm được trong mã gốc iOS

Kiểm tra ngày 27/10/2026. Chưa thay mapping của bản PC. Đây là vị trí gọi trong mã gốc, không phải tên âm thanh do nghe đoán.

## Chuỗi chỉ số đã đối chiếu

- Audio::init (0x660dc) tạo 134 SoundFile với chỉ số 0..133; con trỏ mỗi phần tử dài 4 byte.
- SoundFile constructor 0x673f8 lưu chỉ số ở offset +8. SoundFile::play 0x6744c truyền chỉ số đó tới PMAudio::load/play.
- PMAudioPackage::ReadPlayables 0x44e02c lưu theo thứ tự record; LoadPlayable 0x44ecc8 truy cập bằng chỉ số này.
- PAC: ordinal 0 là module; ordinal 1..133 tương ứng sample 8..140. Resource ID trong record là 141..274, không phải chỉ số gọi API. Vì thế offset 0x208 / 4 = 130 → sample 137, không phải sample 130.
- Bảng đối chiếu: `analysis/ios/sfx_playable_ordinals.json`; mã disassembly: `sfx_index_chain.txt`, `sfx_mapping_audit.txt`, `audio_decode.txt` trong cùng thư mục.

## Vị trí gọi có ID cố định

| Nội dung / hàm gốc | API ordinal | Sample nghe thử | Bằng chứng |
|---|---:|---|---|
| Chọn công trình — PlaceableItem::setSelected | 128 | SFX-135 | SoundFile ở offset 0x200 |
| Gắn công trình vào ô xây — Attraction::attachToBuildSlot | 86 | SFX-093 | 0xa043a → 0xa0440, offset 0x158 |
| Xác nhận xóa công trình — Attraction::confirmDelete | 77 | SFX-084 | 0x9955c truyền 0x4d tới Audio::playAt |
| Hoàn tất profiteering — Attraction::completeProfiteering | 124 | SFX-131 | 0x9ec38 → 0x9ec3e, offset 0x1f0 |
| Nút giao diện — UIButton::process, một nhánh | 130 | SFX-137 | 0x1450f0 → 0x1450f6, offset 0x208 |
| Nút giao diện — UIButton::process, nhánh khác | 120 | SFX-127 | 0x145238 → 0x14523e, offset 0x1e0 |
| Mở territory — ThemeSlot::unlockTerritory | 80 | SFX-087 | offset 0x140 |
| Construction::update | 91 | SFX-098 | offset 0x16c; chưa kết luận thời điểm trong tiến trình xây |
| Thuê nhân viên — StaffMenu::hireStaff | 95 | SFX-102 | offset 0x17c |
| Mở bộ phận nhân viên — StaffMenu::unlockDepartment | 84 | SFX-091 | offset 0x150 |
| MiniTips::update | 129 | SFX-136 | offset 0x204; chưa kết luận điều kiện nhánh |

SFX-131 được dùng chung tại completeInstantly, tryCompleteUpgradeInstantly, doUnlockTerritory và mở prompt; không đặt tên riêng là “thu tiền” cho mọi tình huống. Hai nhánh UIButton còn cần truy điều kiện trước khi gọi một âm là “nhấn thành công” hoặc “lỗi”.

## Phần chưa xác minh

Attraction::playRandomSfx và tryPlayRandomMusic chọn chỉ số động; SoundEvent giữ một ID do caller cấp rồi phát theo vị trí mô hình. Vì vậy chưa thể gán tất cả 141 sample cho từng ride chỉ từ danh sách file âm thanh. Cần lần tiếp cấu hình/caller và điều kiện kích hoạt, giới hạn lặp, khoảng cách, âm lượng.

Các tên click/tap/build/collect/upgrade/sell/board/complete trong bản PC trước đây là gán tạm. Chúng không được lấy từ những liên kết gốc trên. Bằng chứng mới cho thấy cần rà lại mapping và điều kiện phát, không chỉ đổi tên file.
