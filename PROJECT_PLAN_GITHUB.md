# Project Plan — Milestones M0 → M10 (Full Detail)

Team of 3 Devs: **A = Graph**, **B = Planner**, **C = IO/CLI/UI**  
Legend: ✅ Done · 🔄 In Progress · ⏳ Pending

---

## M0 — Bootstrap & CI

**CMakeLists.txt**  
**Mục đích:** cấu hình build lib planner, exe planner_cli, và test.  
**Làm gì:** Khai báo C++20, add target planner (chưa có nhiều source), add target test (gtest), enable_testing, add planner_cli. Thiết lập include directories cho src/.  
**AC:** `cmake && ctest` chạy xanh local & CI.

**.github/workflows/ci.yml**  
**Mục đích:** CI build + test trên Ubuntu.  
**Làm gì:** bước checkout → configure → build → chạy ctest (show log when fail).  
**AC:** Workflow xanh khi push/PR.

**.gitignore**  
**Mục đích:** tránh commit build/IDE file.  
**Làm gì:** thêm build/, .vscode/, *.obj, *.o, *.pdb,…  
**AC:** `git status` không hiện file rác.

**README.md**  
**Mục đích:** hướng dẫn build/chạy nhanh + sơ đồ cấu trúc thư mục.  
**Làm gì:** mục tiêu dự án; lệnh build; lệnh chạy CLI demo; cấu trúc folders.  
**AC:** Người mới clone làm theo được ngay.

---

## M1 — Data Schema & Loader

**src/model/Course.h**  
**Mục đích:** mô tả 1 môn học.  
**Làm gì:** khai báo trường bắt buộc (id, name, credits, prereq, coreq) + optional (group, offered_terms, priority).  
**Edge:** credits > 0, id non-empty.  
**AC:** đủ trường để tạo đồ thị & ràng buộc planner.

**src/model/Curriculum.h**  
**Mục đích:** tập hợp các Course.  
**Làm gì:** map id → Course; lookup O(1).  
**AC:** truy cập theo id ổn định; iterate all.

**src/model/PlanConstraints.h**  
**Mục đích:** cấu hình lập kế hoạch.  
**Làm gì:** max/min credits per term, numTerms, enforceCoreqTogether… kèm default.  
**AC:** dùng được cho assigner không cần thêm config.

**src/io/Loader.h / Loader.cpp**  
**Mục đích:** API load JSON & validate mạnh tay.  
**Làm gì:** Đọc file, parse JSON, populate Course/Curriculum/PlanConstraints. Validate thiếu field, id trùng, credits ≤ 0, prereq/coreq không tồn tại, offered_terms ngoài [1..numTerms]. Ném std::runtime_error có mã lỗi + ngữ cảnh.  
**Test gợi ý:** Loader_Valid_Sample_OK, Loader_MissingField_Fails, Loader_DuplicateId_Fails, Loader_UnknownPrereq_Fails, Loader_Offered_OutOfRange_Fails.  
**AC:** test pass; message rõ.

**tests/loader_valid_test.cpp & loader_invalid_test.cpp**  
**Mục đích:** kiểm định Loader.  
**Làm gì:** 3–5 case valid/invalid, cover ≥90% nhánh validate.

**data/sample_small.json, data/invalid_missing_field.json**  
**Mục đích:** dữ liệu vào test.  
**Làm gì:** tạo 1 dataset hợp lệ (4–8 môn) + 1 dataset lỗi.  
**AC:** dùng cho test.

---

## M2 — Graph & TopoSort + Cycle

**src/graph/CourseGraph.h / .cpp**  
**Mục đích:** định nghĩa cấu trúc đồ thị & buildGraph.  
**Làm gì:** Graph{n, adj, indeg, idOf, idxOf}; duyệt courses, thêm cạnh prereq, tính indeg. (Bỏ coreq/elective, sẽ thêm ở M5.)  
**AC:** số cạnh/indeg đúng; build không warning.

**src/graph/TopoSort.h / .cpp**  
**Mục đích:** thuật toán topo (Kahn).  
**Làm gì:** TopoResult{bool success, vector<int> order}; queue indeg=0; success=false nếu còn đỉnh chưa lấy.  
**Edge:** node cô lập, multi-sources, multi-sinks.  
**AC:** DAG trả order đủ V phần tử.

**src/planner/CycleDiagnosis.h / .cpp**  
**Mục đích:** phát hiện & liệt kê vòng.  
**Làm gì:** DFS/back-edge hoặc stack state → trả ≥1 cycle.  
**AC:** khi topo fail, liệt kê ≥1 vòng đúng.

**tests/graph_topo_test.cpp**  
5–6 case DAG + 2 case cycle → pass toàn bộ.

**data/branching.json, data/cycle.json**  
Dataset DAG phân nhánh & đồ thị có vòng.

---

## M3 — Longest Path (DAG) → Earliest Term

**src/planner/LongestPathDag.h / .cpp**  
**Mục đích:** tính earliestTerm (1-based).  
**Làm gì:** level[v] = 1 + max(level[u]) với u→v; mặc định 0; trả level+1.  
**Edge:** nhiều nguồn, node cô lập.  
**AC:** term đúng mọi case.

**tests/earliest_term_test.cpp**  
3–4 case: ETerm_Branching_OK, ETerm_MultiSources_OK, ETerm_Disconnected_OK.

---

## M4 — TermAssigner v1 (Quota tín chỉ)

**src/planner/PlanTypes.h**  
Term{index, courseIds, credits}; PlanResult{feasible, terms, notes}.

**src/planner/TermAssigner.h / .cpp**  
Greedy theo earliestTerm; nếu vượt quota → đẩy kỳ sau; tie-break out-degree cao trước.  
**Edge:** quota chặt khiến dồn kỳ.  
**AC:** không kỳ nào vượt quota; phụ thuộc đúng.

**src/cli/main.cpp**  
CLI demo: parse build -i file → Loader→Graph→Topo→ET→Assigner → in tổng số kỳ/tín chỉ.

**tests/assigner_quota_test.cpp**  
3 case cơ bản.

---

## M5 — Corequisite & Elective Groups

**src/planner/Clusterizer.h / .cpp**  
Xây tập cluster coreq (connected components); tổng credits cluster; nếu > quota → infeasible. Trả mapping course→cluster.  
**Edge:** coreq vòng/chuỗi; cluster chạm quota.  
**AC:** cluster tạo đúng; detect infeasible.

**src/planner/ElectiveResolver.h / .cpp**  
Chọn-k môn trong mỗi group electives theo priority/credits; loại môn không chọn.  
**Edge:** không đủ môn để chọn; prereq thiếu.  
**AC:** danh sách chọn cuối cùng hợp lệ.

**tests/coreq_electives_test.cpp**, **data/coreq_small.json**, **data/elective_choose2.json**  
Test coreq + electives pass.

---

## M6 — Offered Terms & Infeasible Hints

**src/planner/Hints.h / .cpp**  
Kiểm tra lý do fail (term đóng/quota/cluster) → sinh notes: tăng numTerms, maxCredits, bỏ coreqTogether, đổi môn nhóm X.  
**AC:** PlanResult.feasible=false có ≥1 note cụ thể.

**(mở rộng) src/cli/main.cpp**  
Thêm flags --terms, --max-cred, --min-cred, --enforce-coreq; in notes/hints đẹp.

**tests/offered_terms_hints_test.cpp**, **data/offered_terms_wintersummer.json**  
Xếp đúng kỳ mở; khi fail → gợi ý đúng.

---

## M7 — Writer (JSON/Markdown) & CLI hoàn thiện

**src/io/Writer.h / .cpp**  
Xuất PlanResult ra JSON & Markdown (bảng Term | Courses | Credits + notes).  
**AC:** file đọc lại được, Markdown render chuẩn.

**(mở rộng) src/planner/Explain.h / .cpp**  
Giải thích vì sao môn ở kỳ k: tìm path dài nhất, trả list id theo thứ tự.

**(mở rộng) src/cli/main.cpp**  
Hoàn thiện lệnh: validate, build -o, explain.

**tests/writer_explain_test.cpp**  
Case JSON round-trip, Markdown đủ cột, Explain path đúng, feasible=false có Notes.  
**AC:** test xanh cả trên CI.

---

## M8 — Quality & Benchmark

**data/sample_medium.json**  
Dataset 60–80 môn, 300–600 cạnh, offered_terms lắt léo, không cycle.  
**AC:** build + plan < 0.5s trên CI.

**bench/README.md**  
Hướng dẫn chạy benchmark; tiêu chí ≤0.5s.

**(tuỳ chọn) .clang-tidy**, **tests/boundary_cases_test.cpp**  
Boundary: ZeroCourses_OK, SingleCourse_OK, LongChain_100_OK, StarShape_OK.

---

## M9 — Visualization & What-if (Optional)

**src/viz/DotExport.h / .cpp**  
Xuất Graphviz DOT; tô màu theo term; label node = id\n(credits).  
**AC:** dot -Tpng không lỗi.

**scripts/render_dot.sh**  
Script nhanh tạo PNG/SVG.

**tests/viz_whatif_test.cpp**, **data/graph_for_viz.json**  
Verify DOT hợp lệ, chạy 2 cấu hình what-if → so sánh số kỳ/variance.

---

## M10 — UI mỏng (Optional, Web/React)

**ui/README.md**  
Hướng dẫn chạy UI: npm i, npm run dev, upload curriculum.json, export plan.

**ui/src/pages/App.*, ui/src/components/PlanTable.*, ui/src/components/CreditsBarChart.***  
App: upload + render; PlanTable: bảng Term|Courses|Credits; Chart: tín chỉ/kỳ.  
**Edge:** infeasible → hiện Notes; file sai schema → báo lỗi.  
**AC:** load sample → hiển thị đúng, tương tác mượt.

---

