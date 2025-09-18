/*API tính earliest term mỗi node dựa trên longest path trong DAG.

Nhận: Graph + topo order; Trả: vector term 1-based.

AC: Với DAG mẫu phân tầng, kết quả kỳ khớp kỳ vọng.

(tiếp theo bạn sẽ làm) src/planner/TermAssigner.h / .cpp

API xếp môn vào kỳ dựa trên earliest term + quota tín chỉ.

Heuristic đơn giản: theo topo, nhảy kỳ khi quá quota.

AC: Không vượt quota; không vi phạm phụ thuộc.

(tuỳ chọn) src/planner/ElectiveResolver.h / .cpp

Chọn k môn trong mỗi nhóm tự chọn trước khi xếp.

AC: Đảm bảo đủ k môn/nhóm; nếu không đủ báo infeasible.

(tuỳ chọn) src/planner/Clusterizer.h / .cpp

Gom coreq thành cluster nếu ràng buộc yêu cầu “cùng kỳ”.

AC: Cluster không vượt quota; nếu vượt → gợi ý.

(tuỳ chọn) src/planner/Hints.h / .cpp

Sinh gợi ý khi infeasible: tăng numTerms, tăng quota, nới coreq, đổi elective.

AC: Khi không xếp được, PlanResult có notes hữu ích.

(tuỳ chọn) src/planner/Explain.h / .cpp

Truy vết đường lý do (đường dài nhất tới một môn), dùng để explain.

AC: Trả chuỗi môn từ gốc → target khớp phụ thuộc dài nhất.*/