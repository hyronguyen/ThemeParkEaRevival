# Kiểm tra 11 ride — 28/10/2026

## Kết quả

Đã bổ sung clip vận hành cho cả 11 ride. Trước đó 425, 427, 522 dùng clip idle/nước; 8 ID còn lại chưa có operating clip trong playtest.

Các clip lấy từ asset gốc. Liên kết model → group được suy luận bằng cây node, nhánh xe/locator, chuyển động và kiểm tra hình ảnh; chưa phải mapping AttractionStruct/server được chứng minh từ mã game. Không chỉ ghép theo số lượng node.

| Ride/ID | Group | Chuyển động sau sửa | Node đổi tại 6s | Điểm khách đổi tại 6s |
|---|---:|---|---:|---:|
| 460 | 89 | Xe chạy theo đường ray xoắn; 8 điểm khách theo xe. | 10 | 8 |
| 425 | 46 | Water Slides: nhân vật trượt theo máng, hiệu ứng nước cùng clip vận hành. | 26 | 5 |
| 461 | 90 | Xe chạy theo đường ray vòng; 9 điểm khách theo xe. | 12 | 9 |
| 520 | 156 | Xe chạy theo đường ray; 8 điểm khách theo xe. | 12 | 8 |
| 418 | 39 | Xe và các bộ phận chuyển động trên đường ray tuyết; 8 điểm khách theo xe. | 22 | 8 |
| 522 | 158 | Log Flume: thuyền chạy theo dòng/máng nước; 12 điểm khách theo thuyền. | 52 | 12 |
| 420 | 41 | Thuyền và các xúc tu quái vật biển chuyển động; 3 điểm khách theo bộ phận. | 56 | 3 |
| 427 | 48 | Skull River: thuyền chạy theo dòng sông; 10 điểm khách theo thuyền. | 25 | 10 |
| 521 | 157 | Xe chạy theo đường đua; 4 điểm xe/khách chuyển động. | 6 | 4 |
| 452 | 81 | Diễn viên/đạo cụ biểu diễn; 16 điểm khán giả giữ nguyên vị trí. | 21 | 0 |
| 453 | 82 | Diễn viên/đạo cụ sân khấu miền Tây; 16 điểm khán giả giữ nguyên vị trí. | 81 | 0 |

## Điều khiển và nghiệm thu

- Chỉ chạy khi có khách; hết khách trả về đầu clip và không tăng thời gian animation.
- NPC dùng ma trận locator cùng cây chuyển động của công trình.
- Minigame dùng cơ chế chung: fill khách, chơi 30 giây, tap tăng tốc animation.
- 452/453 là sân khấu: khán giả đứng yên là đúng; chuyển động nằm ở diễn viên/đạo cụ.
- Native test lấy 24 mẫu xuyên chu kỳ: root không dịch chuyển, toàn bộ ma trận hữu hạn; kiểm tra đủ khách, có node chuyển động và dừng khi hết khách.
- Lưu ảnh mỗi ride tại 0s, 6s, 14s; đã xem contact sheet.
- Báo cáo máy: [requested-rides-animation.json](../analysis/runtime_test/requested-rides-animation.json).
- Ảnh tổng: [requested-rides-contact.jpg](../analysis/runtime_test/requested-rides-contact.jpg).

## Dữ liệu tái lập

Binding: `extracted/runtime/operating_animations.tsv`. Native self-test: `src/themepark_3d.cpp`. Loader/điều khiển: `src/park_game.h`.

| Group | SHA-256 clip |
|---|---|
| 89 | `77a21e9b2696b2b3455e1c3711e6d5c33aeb14ac29fb16af44dfcfab76d9140c` |
| 46 | `df93bf906a1a1c04c2e855723a2dc98e14902c3aee170320fde19cd0965aec1a` |
| 90 | `88c56fcc7fa6c8c4dadd2e7e1c4d0913a38be81e77951f719847679a0756bacd` |
| 156 | `0714b9739366b2c0a86010cc367341b0344d4f0bea744ad840917a515b540eb4` |
| 39 | `a3df4f3b8c2b907575289f2ce49d4bf83fa6190eba2629688b7557ee4ee27cc8` |
| 158 | `3666e91d49514da80d8b5809fd8c2e15239388777117c2c07cfa0e448591eeda` |
| 41 | `64fce6f62e7692de7f0f5c2daa0f5c181e13cc08bf25c17fbc5f05692b223530` |
| 48 | `be3780192cd1fb0ecc59ec6d51b4cc659a7586e13de48124e5b7178370ae59a7` |
| 157 | `372d1ce01cdd10679d1caaf5a9114ea6eabf4a841fa02268dabbc1cb738ce6be` |
| 81 | `1fffbb7d760256896abd186bc4c277a246f32deead87121e359961e8a3925862` |
| 82 | `f65b117176d569a6f7d1007679b60997f3bbb648480ff8e7b0207e465509835e` |

Kiểm thử cuối: 49 Python tests, C++ simulation và native self-test đều đạt trên `build/ThemeParkGame.exe` (SHA-256 `6a0c123cae208c0d3a5299d436114740af6e3704d8bdee4b07c02bb6fc778fc1`).
