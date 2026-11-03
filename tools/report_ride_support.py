"""Report coverage without equating passenger locators with recovered animation."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
MECHANICAL = {409,410,430,445,449,463,465,467,471,476,511}
ORIGINAL_IDLE = {425,427,515,522}

def main():
    locators = {}
    for line in (ROOT / "extracted/runtime/model_locators.tsv").read_text().splitlines():
        model, node, ordinal = map(int, line.split())
        locators.setdefault(model, []).append(node)
    operating = {int(line.split()[0]) for line in (ROOT / "extracted/runtime/operating_animations.tsv").read_text().splitlines() if line.strip() and not line.startswith("#")}
    rows = []
    for line in (ROOT / "extracted/runtime/attractions.tsv").read_text().splitlines():
        model, category, artwork, name = line.split("\t", 3)
        model, artwork = int(model), int(artwork)
        status = "castle_reference" if model == 404 else "original_operating_validated" if model in operating else "provisional_mechanical" if model in MECHANICAL else "original_idle_only" if model in ORIGINAL_IDLE else "locator_only" if model in locators else "no_binding"
        rows.append(dict(model=model, name=name, category=category, artwork=artwork, shop_visible=bool(artwork), seats=len(locators.get(model, [])), status=status))
    (ROOT / "analysis/ride_support.json").write_text(json.dumps(rows, indent=2), encoding="utf-8")
    labels = {"original_operating_validated":"Clip vận hành gốc; binding suy luận đã kiểm tra native", "castle_reference":"Nhà phao chuẩn tham chiếu", "provisional_mechanical":"Chuyển động cơ khí tạm + NPC theo khớp", "original_idle_only":"Clip idle gốc; chưa đủ animation vận hành", "locator_only":"NPC có điểm gắn; chưa có animation vận hành", "no_binding":"Chưa có liên kết animation/NPC được xác minh"}
    lines = ["# Danh sách animation từng ride — 27/10/2026", "", "Sinh bằng `python tools/report_ride_support.py`. Không suy ra hỗ trợ animation từ việc chỉ có locator.", "", "## Tiêu chí Bouncy Castle", "", "Ride trống đứng yên; có khách mới chạy; NPC đi cùng điểm chơi; tap tăng tốc; khách vào/ra tức thì; không hàng chờ. Chuyển động cơ khí tạm đáp ứng điều kiện điều khiển nhưng chưa được chứng minh giống clip gốc. Bốn clip idle không đồng nghĩa hoàn tất chuyển động chơi hoặc đường đi của xe.", "", "Bổ sung ngày 28/10: 11 operating clips đã kiểm tra; xem [báo cáo từng ride](requested-rides-animation-2026-10-28.md). Binding operating được suy luận từ hierarchy/chuyển động và kiểm tra hình ảnh, không phải mapping server đã phục hồi.\n\n## Ride hiện trong shop", "", "| ID | Ride | Số điểm NPC | Trạng thái |", "|---|---|---:|---|"]
    for r in rows:
        if r["category"] == "RIDES" and r["shop_visible"]:
            lines.append(f'| {r["model"]} | {r["name"]} | {r["seats"]} | {labels[r["status"]]} |')
    lines += ["", "## Mô hình hỗ trợ thêm nhưng ẩn khỏi shop", "", "511: quay cụm tay bạch tuộc, NPC theo ghế; chưa có artwork chi tiết nên không đưa vào shop.", "", "## Kiểm tra và phần còn thiếu", "", "Native self-test kiểm tra 11 ride cơ khí: bộ phận và locator đổi ma trận, root đứng yên, không khách trả về bind pose. Có ảnh hai thời điểm cho 409, 449, 467, 476, 511. Ferris Wheel 476 có bù góc cabin để giữ khách thẳng đứng.", "", "Chưa nghiệm thu toàn bộ danh mục theo chuẩn Bouncy Castle. Cần phục hồi liên kết AttractionStruct → animation vận hành và tư thế NPC cho các dòng chưa hoàn tất; không ghép clip chỉ vì trùng số node. 203 nhóm animation được trích xuất nhưng hiện chỉ 4 liên kết idle có bằng chứng từ mã gốc. Các ID chưa có tên xác minh giữ nguyên Original Ride, không tự đặt tên game.", "", "Chi tiết node mới: 467 nâng node 3 (cụm ghế), 476 quay node 2 quanh Y và bù góc các cabin con, 511 quay node 1 quanh Z. Tốc độ/biên độ là tham số playtest."]
    (ROOT / "docs/ride-animation-coverage-2026-10-27.md").write_text("\n".join(lines)+"\n", encoding="utf-8")
    print("Catalog:", len(rows), "visible rides:", sum(r["shop_visible"] and r["category"]=="RIDES" for r in rows))

if __name__ == "__main__":
    main()
