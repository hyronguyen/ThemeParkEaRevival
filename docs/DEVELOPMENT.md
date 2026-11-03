# Hướng dẫn phát triển và cấu trúc project

## Bắt đầu một phiên

1. Đọc [STATUS.md](STATUS.md), chọn mục trong [ROADMAP.md](ROADMAP.md).
2. Xem báo cáo liên quan qua [analysis/README.md](../analysis/README.md).
3. Đối chiếu archive/video gốc trước khi khẳng định hành vi đã được khôi phục.
4. Sửa, build/test theo phần bị tác động, xem ảnh native; cập nhật trạng thái và giới hạn.

## Các thư mục

| Thư mục | Vai trò | Cách xử lý |
|---|---|---|
| `src/` | Mã native và simulation | Nguồn phát triển chính |
| `tools/` | Điều tra/trích xuất/stage asset, kiểm tra PE | Giữ script tái lập được |
| `scripts/` | Build, kiểm thử, dọn file phát sinh | Chạy từ PowerShell |
| `tests/` | Python và C++ tests | Theo hành vi, không chỉ snapshot mã |
| `docs/` | Trạng thái, roadmap, hướng dẫn này | Điểm vào tài liệu |
| `analysis/` | Báo cáo bằng chứng và manifest | Giữ đường dẫn để tool/reference không hỏng |
| `analysis/runtime_test/` | JSON và ảnh test gần nhất | Có thể được ghi đè khi test |
| `analysis/archive/` | Ảnh nghiên cứu và build cũ đã gom | Có manifest hash/đường dẫn gốc; không dùng làm runtime |
| `resource/` | IPA/APK và video người dùng | Giữ nguyên, chỉ đọc |
| `extracted/` | Asset gốc đã trích và catalog runtime | Game cần thư mục này |
| `build/` | EXE hiện hành, symbol, cache build | Không di chuyển EXE khỏi vị trí mặc định |
| `.tools/`, `tools/_deps/` | Compiler và thư viện điều tra cục bộ | Không phải asset chạy game |

`build/ThemeParkGame.exe` là game; `ThemePark3D.exe` là workbench model/bản đồ;
`ThemeParkPreview.exe` là preview 2D đời đầu. Hai EXE Game/3D dùng cùng binary,
chọn chế độ theo tên file hoặc `--play`. Nếu game đang mở, build có thể tạo
`ThemeParkGame-update.exe`; xem kết quả build để biết file nào vừa cập nhật.

## Bản đồ mã nguồn

| File | Trách nhiệm |
|---|---|
| `src/themepark_3d.cpp` | Win32/WGL, tải model/texture, renderer, input, native self-test |
| `src/park_simulation.h` | Graph/BFS, admission 50%, visit/bus states, save/validation |
| `src/park_game.h` | Ghép world asset, NPC/castle, office/bus, placement, điều phối UI |
| `src/park_original_ui.h` | Atlas/thumbnail/font gốc, HUD và menu RIDES/SHOPS |
| `src/park_audio.h` | WinMM/MCI và ba MP3 gốc |
| `src/themepark_preview.cpp` | Công cụ preview 2D cũ |

Các header game/UI được include sau helper renderer, có phụ thuộc thứ tự; hiện chưa
phải module biên dịch riêng. Không đổi cấu trúc include trong một thay đổi nội dung
nhỏ; nếu tách module hãy làm thành hạng mục riêng và kiểm tra cả hai chế độ Game/3D.

## Build và kiểm tra

Chạy ở gốc project; compiler cục bộ Zig 0.13.0. Không cần trích lại asset mỗi lần.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-playtest.ps1
$env:THEMEPARK_VERIFY_REAL_ASSETS='1'
python -m unittest discover -s tests
python tools/verify_native.py ThemeParkGame.exe
```

Khi sửa renderer/export model, chạy thêm `build/ThemePark3D.exe --self-test` và xem
ảnh trong `analysis/runtime_test/`. Không đóng tiến trình game của người dùng để build.
Trên môi trường sandbox, build/native window test hoặc test Python dùng thư mục tạm
có thể cần quyền chạy ngoài sandbox. Không coi lỗi quyền là lỗi gameplay.

Tái tạo toàn bộ asset: theo chuỗi lệnh trong [README gốc](../README.md), từ inspect,
extract assets/models/animations, export scene/locators đến prepare audio/expansion.
`tools/prepare_expansion.py` và PNG extraction cần Pillow. Python có Pillow trên máy này:

```powershell
& 'C:/Users/kutao/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' tools/prepare_expansion.py
```

## Những ràng buộc dễ làm hỏng bản chơi

- Giữ năm type index đầu: 425, 427, 515, 522, 404. Thêm type ở cuối hoặc viết migration.
- Giữ thứ tự path index cũ; walkway mới đã append để save cũ không bị trỏ nhầm tile.
- Khách không xếp hàng ride; 50% là **xác suất admission**, không phải ngưỡng sức chứa.
- `vis_loc` là điểm gắn khách: giữ transform, không vẽ mesh đánh dấu.
- Tô ô đất phải giữ nguyên GL state và không ghi depth che floor/alpha.
- Mọi việc save dùng schema độc lập; không nhầm với save/account mobile gốc.
- Giữ bản gốc chỉ đọc. Không publish asset/archive/derivative độc quyền hoặc toolchain.

## Dọn kết quả nghiên cứu

```powershell
# Xem số file sẽ gom, chưa thay đổi gì
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/tidy-project.ps1
# Chuyển ảnh catalog/contact-sheet và EXE update cũ vào archive; không xóa
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/tidy-project.ps1 -Apply
```

Script kiểm tra đường dẫn nằm trong project, lưu kế hoạch trước khi di chuyển và
đối chiếu SHA-256 sau khi chuyển. `manifest.json` cho biết chỗ cũ/mới của từng file.
Các chế độ `--ride-sheet`/`--catalog-capture` vẫn có thể sinh ảnh mới tại runtime_test;
chạy tidy lại sau khi kết thúc đợt nghiên cứu. Giữ symbol/cache để hỗ trợ debug và
build nhanh; không tự xóa dữ liệu lưu trong LocalAppData.
