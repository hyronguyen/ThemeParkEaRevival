# Mục lục bằng chứng và báo cáo

**Trạng thái hiện hành:** [docs/STATUS.md](../docs/STATUS.md).
**Hạng mục tiếp theo:** [docs/ROADMAP.md](../docs/ROADMAP.md).
Các báo cáo lịch sử có thể chứa tham số của milestone cũ.

| Chủ đề | Tài liệu |
|---|---|
| UI/catalog/bus/decorations hiện tại | [expansion_ui_gameplay.md](expansion_ui_gameplay.md) |
| Nhà phao, locator, MP3 | [castle_audio_rendering.md](castle_audio_rendering.md) |
| Simulation và save đời đầu | [playtest_mechanics.md](playtest_mechanics.md), [data_structures.md](data_structures.md) |
| Mesh/material/animation | [model_formats.md](model_formats.md), [asset_formats.md](asset_formats.md) |
| Layout, path, map | [scene_formats.md](scene_formats.md) |
| Video tham khảo | [gameplay_reference.md](gameplay_reference.md), `previews/gameplay/` |
| iOS/Android | [ipa_report.md](ipa_report.md), [apk_report.md](apk_report.md), [platform_comparison.md](platform_comparison.md) |
| Engine và hàm gốc | [engine_identification.md](engine_identification.md), [engine_architecture.md](engine_architecture.md), [binary_function_map.md](binary_function_map.md) |
| Gameplay chưa khôi phục | [gameplay_systems.md](gameplay_systems.md), [offline_compatibility.md](offline_compatibility.md) |
| Toolchain và lịch sử test | [toolchain.md](toolchain.md), [test_results.md](test_results.md) |

`ios/`, `android/`: symbol/disassembly và bằng chứng kỹ thuật.
Các JSON lớn ở đây được exporter/tests tham chiếu, nên giữ vị trí hiện tại.
`runtime_test/`: ảnh và báo cáo kiểm thử gần nhất, không phải kho lịch sử bất biến.
`archive/<batch>/manifest.json`: tra ảnh catalog/contact-sheet và EXE cũ đã dọn.
`previews/`: các ảnh tham khảo còn được báo cáo/tool sử dụng; giữ video sample riêng.
