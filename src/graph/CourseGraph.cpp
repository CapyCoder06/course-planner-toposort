/*Xây mapping id → index, mảng idOf.

Tạo cạnh prereq → course.

Tính in-degree cho mỗi node.

AC: Với input mẫu, số đỉnh/cạnh đúng, in-degree hợp lý.

(khuyến nghị) src/graph/TopoSort.h / .cpp

Khai báo & hiện thực thuật toán topo (Kahn/DFS).

Trả thứ tự topo + cờ success (false nếu có vòng).

AC: Đồ thị DAG cho thứ tự hợp lệ; đồ thị có vòng báo fail.

(khuyến nghị) src/planner/CycleDiagnosis.h / .cpp

Phát hiện và liệt kê vòng (danh sách node/cạnh).

AC: Khi topo thất bại, có thể truy ra ít nhất một vòng cụ thể.*/