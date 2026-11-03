"""Publish measured runtime coverage, without claiming inferred bindings are original."""
import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'analysis/runtime_test'

def rows(name):
    with (OUT / name).open(encoding='utf-8') as f:
        return list(csv.DictReader(f, delimiter='\t'))

balance = rows('local-balance.tsv')
names = {r['model']: r['name'] for r in balance}
coverage = rows('operating-coverage.tsv')
report = '''# Animation, bảng giá và ánh sáng — 31/10/2026

## Thay đổi

- Tách clip idle khỏi clip vận hành: Water Slides 425 (47/46), Skull River 427 (49/48), Wave Rider 515 (151/150), Log Flume 522 (159/158). Hiệu ứng nước tiếp tục khi không có khách. Water Slides thêm dịch UV riêng dải tia nước, không dịch vùng atlas của vật trang trí.
- Mở rộng liên kết clip gốc theo thứ tự catalog và cấu trúc node. Đây là mapping suy luận, không phải bảng liên kết máy chủ được khôi phục. 76 ứng viên được xuất; 52 ride hiện trong danh mục đạt kiểm tra runtime. Bouncy Castle 404 giữ cơ chế riêng đã có.
- Mua thực tế dùng giá và thời gian xây mới; danh mục sắp theo giá tăng dần, hiển thị thời gian xây. Bouncy Castle $2.600 / 3s, Teacups $6.300 / 10s, Ferris Wheel $6.800 / 15s. Các mức còn lại là cân bằng tạm theo nhóm kích thước và ID, chưa cân bằng theo thời gian hoàn vốn. Không thay tiền trong save.
- Minigame 20s phát một tiếng khách khi đạt 16,5s (còn 3,5s), tuân theo bật/tắt âm thanh. Dùng mẫu gốc sample 10 của nhóm giọng khách; chưa xác nhận đây là tiếng reo chính xác của trò gốc bằng nghe đối chiếu.
- Thử chu kỳ ngày/đêm 360 giây mô phỏng, thay hướng đèn và ánh sáng môi trường. Settings có nút bật/tắt, tắt trở về ban ngày. Chưa có đổ bóng hoặc đèn điểm riêng; lựa chọn bật/tắt chưa lưu qua lần mở game.

## Giới hạn cần xử lý tiếp

- 414 và 415: chưa tìm được clip/assembly đáng tin cậy; không quay toàn bộ mô hình để giả animation.
- 422: clip ứng viên 43 có transform cực lớn (tới hàng tỷ), bị loại để tránh biến dạng. Chưa khôi phục đầy đủ vận hành.
- Hedge Maze 432: chưa có đường đi nội bộ/clip phù hợp.
- Kiểm tra chuyển động node/ghế không chứng minh mọi tư thế NPC hoặc toàn bộ chu kỳ đã giống bản gốc. Các dòng có 0 ghế chuyển động chỉ xác nhận chuyển động mô hình, chưa xác nhận tương tác NPC di chuyển.

## Kiểm chứng

53 Python tests, simulation C++ và native self-test đạt. Native kiểm tra 52 ride tại 12 thời điểm: root đứng yên, ma trận hữu hạn và có node chuyển động; kiểm tra idle nước và tiếng khách chỉ kích hoạt một lần. Ảnh ngày/đêm, Water Slides và Ferris Wheel đã xem trực tiếp. Các ảnh mẫu bổ sung nằm trong analysis/runtime_test/expanded-motion-*.png.

Executable: build/ThemeParkGame.exe, native AMD64, 1.024.000 byte, SHA-256 `03e4e616b01ef5ed3c78e0ee42857668429e91e77eab8997382864bf87145d5b`.

## Từng ride có clip được kiểm tra

| ID | Tên hiện tại | Clip | Node chuyển động | Locator ghế chuyển động |
|---|---|---:|---:|---:|
'''
for r in coverage:
    report += f"| {r['model']} | {names.get(r['model'], '')} | {r['group']} | {r['moving_nodes']} | {r['moving_seats']} |\n"
report += '\n## Bảng giá đầy đủ\n\nBao gồm cả mục ẩn để giữ tương thích catalog/save. Chỉ mục có artwork đủ điều kiện mới hiện trong shop.\n\n| ID | Tên | Cỡ | Giá tiền | Xây (giây) |\n|---|---|---:|---:|---:|\n'
for r in balance:
    report += f"| {r['model']} | {r['name']} | {r['size']} | {r['cash']} | {r['build_seconds']} |\n"
(ROOT / 'docs/motion-balance-light-2026-10-31.md').write_text(report, encoding='utf-8')
print('Wrote measured coverage and balance report')
