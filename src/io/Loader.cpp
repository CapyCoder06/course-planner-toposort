/*Đọc file JSON → parse thành Course, Curriculum, PlanConstraints.

Validate:

Thiếu trường bắt buộc.

Tín chỉ > 0.

id không trùng.

Tham chiếu prereq/coreq phải tồn tại.

Nếu có offered terms: nằm trong [1..numTerms].

Thông báo lỗi có mã/ngữ cảnh (ví dụ: path khóa bị lỗi).

AC: Input hợp lệ → load ok; input lỗi → ném lỗi rõ ràng.

(tuỳ chọn) src/io/Writer.h / Writer.cpp

API xuất plan ra JSON + Markdown.

AC: Có thể ghi ra terms, tổng tín chỉ/kỳ, và ghi chú.*/