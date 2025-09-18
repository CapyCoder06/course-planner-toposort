# Project Plan -- Milestones M0 to M10 (Full Detail)

# Project Plan -- Milestone M0 to M6 (Detailed)

## M0 --- Bootstrap & CI

CMakeLists.txt\
- [ ] Mục đích: cấu hình build lib planner, exe planner_cli, và test.\
- [ ] Làm gì: Khai báo C++20, add target planner, add target test (gtest),
enable_testing, add planner_cli, thiết lập include directories.\
- [ ] AC: ```bash
cmake && ctest
``` xanh local & CI.\
\
.github/workflows/ci.yml\
- [ ] CI build + test trên Ubuntu.\
- [ ] Làm gì: checkout → configure → build → chạy ctest.\
- [ ] AC: Workflow xanh khi push/PR.\
\
.gitignore\
- [ ] Mục đích: tránh commit build/IDE file.\
- [ ] Làm gì: thêm build/, .vscode/, \*.obj, \*.o, \*.pdb...\
- [ ] AC: git status không hiện file rác.\
\
README.md\
- [ ] Mục đích: hướng dẫn build/chạy nhanh.\
- [ ] Làm gì: mục tiêu dự án, lệnh build, lệnh chạy CLI demo, cấu trúc
folders.\
- [ ] AC: người mới clone làm theo được ngay.

## M1 --- Data Schema & Loader

src/model/Course.h\
- [ ] Mục đích: mô tả 1 môn học.\
- [ ] Làm gì: khai báo id, name, credits, prereq, coreq + optional group,
offered_terms, priority.\
- [ ] Edge: credits\>0, id non-empty.\
- [ ] AC: đủ trường để tạo đồ thị.\
\
src/model/Curriculum.h\
- [ ] Map id → Course.\
- [ ] AC: truy cập O(1), iterate all.\
\
src/model/PlanConstraints.h\
- [ ] max/min credits per term, numTerms, enforceCoreqTogether (default).\
- [ ] AC: dùng được cho assigner.\
\
src/io/Loader.h/.cpp\
- [ ] LoadResult{Curriculum, PlanConstraints}.\
- [ ] Đọc JSON, populate, validate thiếu field, id trùng, credits≤0,
prereq/coreq không tồn tại, offered_terms ngoài range.\
- [ ] Ném std::runtime_error có mã + ngữ cảnh.\
- [ ] Test gợi ý: Loader_Valid_Sample_OK, MissingField_Fails,
DuplicateId_Fails, UnknownPrereq_Fails, Offered_OutOfRange_Fails.\
- [ ] AC: pass toàn bộ test.\
\
tests/loader_valid_test.cpp & loader_invalid_test.cpp\
- [ ] Test 3--5 case valid/invalid, cover ≥90%.\
\
data/sample_small.json, data/invalid_missing_field.json\
- [ ] Dataset mẫu và dataset lỗi.\
- [ ] AC: dùng được cho test.

## M2 --- Graph & TopoSort + Cycle

src/graph/CourseGraph.h/.cpp\
- [ ] Định nghĩa Graph{n, adj, indeg, idOf, idxOf}; buildGraph từ
Curriculum.\
- [ ] Test: số đỉnh/cạnh & indeg.\
\
src/graph/TopoSort.h/.cpp\
- [ ] Kahn topo: trả TopoResult{success, order}.\
- [ ] Edge: node cô lập, multi-sources, multi-sinks.\
- [ ] Test: Topo_Line_OK, Topo_Branching_OK, Topo_Isolated_OK.\
\
src/planner/CycleDiagnosis.h/.cpp\
- [ ] DFS/back-edge tìm ≥1 cycle.\
- [ ] Test: Cycle_Simple_ABA, Cycle_TwoDisjoint.\
\
tests/graph_topo_test.cpp\
- [ ] Gom test graph, topo, cycle.\
\
data/branching.json, data/cycle.json\
- [ ] Dataset DAG và có vòng.\
\
AC: DAG trả order đủ; có vòng → success=false và liệt kê ≥1 vòng.

## M3 --- Longest Path (DAG) → Earliest Term

src/planner/LongestPathDag.h/.cpp\
- [ ] Input: Graph + topoOrder.\
- [ ] level\[v\] = 1 + max(level\[u\]) với cạnh u→v.\
- [ ] Edge: nhiều nguồn, node cô lập.\
- [ ] Test: ETerm_Branching_OK, ETerm_MultiSources_OK,
ETerm_Disconnected_OK.\
- [ ] AC: term đúng mọi node.\
\
tests/earliest_term_test.cpp\
- [ ] 3--4 case.\
- [ ] AC: pass.

## M4 --- TermAssigner v1 (Quota)

src/planner/PlanTypes.h\
- [ ] Term{index, courseIds, credits}; PlanResult{feasible, terms, notes}.\
\
src/planner/TermAssigner.h/.cpp\
- [ ] Greedy assign theo topo + earliestTerm.\
- [ ] Nếu vượt quota → đẩy kỳ sau.\
- [ ] Tie-break out-degree cao trước.\
- [ ] Test: RespectsQuota, ShiftWhenFull, KeepsTopoOrder.\
\
src/cli/main.cpp\
- [ ] CLI tối thiểu build pipeline.\
\
tests/assigner_quota_test.cpp\
- [ ] 3 case.\
\
AC: không kỳ nào vượt quota; phụ thuộc giữ đúng thứ tự.

## M5 --- Corequisite & Elective Groups

src/planner/Clusterizer.h/.cpp\
- [ ] Gom coreq thành cluster.\
- [ ] Tổng credits cluster; nếu \> quota → infeasible.\
- [ ] Mapping course→cluster cho assigner.\
- [ ] Edge: coreq vòng/chuỗi, cluster chạm quota.\
- [ ] Test: Coreq_SameTerm_OK, Coreq_TooManyCredits_Fails.\
- [ ] AC: cluster tạo đúng, detect infeasible khi cần.\
\
src/planner/ElectiveResolver.h/.cpp\
- [ ] Chọn-k môn theo priority/credits.\
- [ ] Loại bỏ môn không chọn.\
- [ ] Test: Electives_ChooseK_OK, Electives_NotEnough_Fails.\
- [ ] AC: chọn đúng, nhất quán prereq.\
\
tests/coreq_electives_test.cpp\
- [ ] Test chung coreq + electives.\
\
data/coreq_small.json, data/elective_choose2.json\
- [ ] Dataset test.\
- [ ] AC: pass.

## M6 --- Offered Terms & Hints

src/planner/Hints.h/.cpp\
- [ ] Sinh gợi ý khi fail: tăng numTerms/maxCredits, relax coreq, đổi
elective.\
- [ ] AC: PlanResult.feasible=false → notes cụ thể.\
\
src/cli/main.cpp (mở rộng)\
- [ ] Thêm flags: \--terms, \--max-cred, \--min-cred, \--enforce-coreq.\
- [ ] In bảng tổng hợp + notes.\
\
tests/offered_terms_hints_test.cpp\
- [ ] Verify offered_terms và gợi ý.\
\
data/offered_terms_wintersummer.json\
- [ ] Dataset có môn chỉ mở một số kỳ.\
- [ ] AC: xếp đúng kỳ mở; khi fail sinh gợi ý.

# Project Plan -- Milestone M7 to M10 (Detailed)

## M7 --- Writer (JSON/Markdown) & CLI hoàn thiện

Mục đích: Xuất kết quả đẹp, CLI đủ lệnh validate/build/explain.

### tests/writer_explain_test.cpp

Mục đích: kiểm chứng Writer (JSON/MD) và Explain chạy đúng format & nội
dung.\
\
Làm gì:\
- [ ] Case 1 (Writer JSON): dựng một PlanResult nhỏ (2--3 term, có notes).
Gọi Writer JSON → parse lại JSON → so sánh trường: terms\[i\].index,
terms\[i\].courses\[\], terms\[i\].credits, notes\[\].\
- [ ] Case 2 (Writer MD): gọi Writer Markdown → assert chuỗi có tiêu đề
bảng, đủ số dòng term, tổng tín chỉ hiển thị đúng.\
- [ ] Case 3 (Explain Path): tạo đồ thị nhỏ có 2 đường đến cùng môn; ensure
explain trả đường dài nhất (hoặc quy ước bạn đặt). Kiểm tra thứ tự id
trong path.\
- [ ] Case 4 (Invalid): PlanResult.feasible=false → JSON có feasible=false,
MD có mục "Notes".\
\
Edge/Cases:\
- [ ] Term rỗng (0 môn) vẫn có dòng hiển thị credits=0.\
- [ ] Notes nhiều dòng, ký tự đặc biệt (kiểm MD escape hợp lý).\
\
AC:\
- [ ] JSON round-trip không mất dữ liệu; Markdown có đủ cột; explain path
đúng (theo longest path/quy ước).\
- [ ] Test chạy xanh cả khi chạy trên CI.

## M8 --- Quality & Benchmark

data/sample_medium.json\
- [ ] Mục đích: dataset cỡ vừa (≈60--80 môn, 300--600 cạnh) để benchmark &
regression test.\
- [ ] Làm gì: nhiều nhánh sâu, bottleneck, môn tín chỉ lớn, offered_terms
phức tạp nhẹ, không có cycle.\
- [ ] AC: build & plan dưới 0.5s trên GitHub Actions.\
\
bench/README.md\
- [ ] Mục đích: hướng dẫn benchmark thủ công/CI.\
- [ ] Làm gì: mô tả lệnh build+run, tiêu chí ≤0.5s.\
- [ ] AC: người khác đọc xong tự chạy đo được.\
\
(tuỳ chọn) .clang-tidy / cấu hình static analysis\
- [ ] Mục đích: bật rule modernize/readability.\
- [ ] AC: tidy chạy không lỗi nghiêm trọng.\
\
(tuỳ chọn) tests/boundary_cases_test.cpp\
- [ ] ZeroCourses_OK, SingleCourse_OK, LongChain_100_OK, StarShape_OK.\
- [ ] AC: pass tất cả.

## M9 --- Visualization & What-if (Optional)

src/viz/DotExport.h / .cpp\
- [ ] Mục đích: xuất Graphviz DOT để vẽ DAG, tô màu theo term.\
- [ ] Làm gì: API nhận Graph, PlanResult → sinh DOT, tô màu theo term, label
node=id/credits.\
- [ ] Edge: course infeasible tô màu riêng, node cô lập vẫn xuất.\
- [ ] AC: .dot mở bằng dot -Tpng không lỗi.\
\
scripts/render_dot.sh\
- [ ] Mục đích: script nhanh tạo PNG/SVG.\
- [ ] AC: chạy 1 lệnh ra ảnh.\
\
tests/viz_whatif_test.cpp\
- [ ] Mục đích: test hợp lệ DOT & what-if.\
- [ ] Làm gì: verify có digraph, đủ node, chạy 2 cấu hình so sánh số
term/variance.\
- [ ] AC: pass.\
\
data/graph_for_viz.json\
- [ ] Dataset nhỏ 8--12 môn, 3--4 kỳ, có bottleneck.\
- [ ] AC: tạo ảnh đẹp, dễ thuyết trình.

## M10 --- UI mỏng (Optional, Web/React)

ui/README.md\
- [ ] Mục đích: hướng dẫn chạy UI.\
- [ ] Làm gì: npm i, npm run dev, upload curriculum.json, xuất
plan.json/md.\
- [ ] AC: người mới làm theo 5 phút chạy được.\
\
ui/src/pages/App.\*, ui/src/components/PlanTable.\*,
ui/src/components/CreditsBarChart.\*\
- [ ] App: upload + render.\
- [ ] PlanTable: bảng Term \| Courses \| Credits.\
- [ ] CreditsBarChart: biểu đồ cột credits/kỳ.\
- [ ] Edge: Plan infeasible → hiện Notes; schema sai → toast lỗi.\
- [ ] AC: sample load đúng, hiển thị mượt.