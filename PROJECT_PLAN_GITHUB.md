# Final Term Project — Application of Topological Sorting for Full‑Course Study Planning

**Course:** Discrete Mathematics and Graph Theory
**University:** HCMUTE — Faculty of Information Technology
**Team:**

* **Dev A – Huỳnh Thuyên Nam:** Graph & Core Algorithms (buildGraph, topo, cycle, dot export)
* **Dev B – Phạm Nguyễn Vương Quốc:** Planner Logic (earliestTerm, assigner, cluster, elective, hints, explain)
* **Dev C – Hà Duy Anh:** IO/CLI/UI, Loader/Writer, Test Harness, CI/CD

**Legend:** ✅ Done · 🔄 In Progress · ⏳ Pending

---

## M0 — Bootstrap & CI

**Goal:** Dựng skeleton, build + test xanh local & CI.

### Owners & Tasks

* **Dev C (owner):** CMake skeleton, CI workflow, README, .gitignore.
* **Dev A:** Dummy domain struct + dummy test để CI chạy xanh.
* **Dev B:** Review cấu trúc, góp ý build flags & layout.

### Folders & Files to Create

```
/ (repo root)
├─ CMakeLists.txt
├─ .gitignore
├─ README.md
├─ .github/workflows/ci.yml
├─ src/
│  ├─ model/        (empty placeholders)
│  ├─ io/           (empty placeholders)
│  ├─ graph/        (empty placeholders)
│  ├─ planner/      (empty placeholders)
│  └─ cli/
│     └─ main.cpp   (Hello Planner)
└─ tests/
   └─ dummy_test.cpp
```

**Mục đích & Làm gì**

* **CMakeLists.txt**: Khai báo C++20, add lib `planner`, exe `planner_cli`, enable\_testing, link gtest.
* **.github/workflows/ci.yml**: Ubuntu latest → checkout → configure (CMake) → build → ctest (show logs when fail).
* **.gitignore**: `build/`, `.vscode/`, `*.o`, `*.obj`, `*.pdb`, `*.DS_Store`.
* **src/cli/main.cpp**: In ra `Hello Planner` (kiểm tra biên dịch & chạy).
* **tests/dummy\_test.cpp**: `EXPECT_EQ(1,1)` để xác nhận hạ tầng test hoạt động.

### Checklist

* [ ] Build được `planner_cli` (runs: `Hello Planner`).
* [ ] `ctest` xanh local.
* [ ] CI xanh trên PR/push.
* [ ] README có hướng dẫn build/run ngắn gọn.

---

## (Optional) M0.5 — UI Skeleton (Static + Mock)

**Goal:** Có giao diện tĩnh sớm để hình dung, tách biệt với core.

### Owners & Tasks

* **Dev C (owner):** Vite+React+TS+Tailwind; components `PlanTable`, `CreditsBarChart`, `NotesPanel`; adapter dùng mock JSON.
* **Dev B:** Viết `plan.contract.md` mô tả JSON `PlanResult` (feasible, terms, notes, explain?).
* **Dev A:** Chuẩn hoá format mã môn (ID rules) để render đẹp.

### Folders & Files

```
/ui/
├─ README.md
├─ package.json
├─ vite.config.ts
├─ src/
│  ├─ main.tsx, App.tsx
│  ├─ types/plan.ts
│  ├─ adapters/plannerAdapter.ts
│  ├─ mocks/plan.sample.json
│  └─ components/
│     ├─ PlanTable.tsx
│     ├─ CreditsBarChart.tsx
│     └─ NotesPanel.tsx
/docs/plan.contract.md
```

**Mục đích & Làm gì**

* **types/plan.ts**: định nghĩa `PlanResult`, `Term`.
* **mocks/plan.sample.json**: fixture để UI hiển thị.
* **plannerAdapter.ts**: load mock giờ; sau chuyển sang output thật từ CLI (không đổi UI).
* **PlanTable/CreditsBarChart/NotesPanel**: bảng Term|Courses|Credits, biểu đồ tín chỉ/kỳ, ghi chú.

### Checklist

* [ ] `npm run dev` hiển thị bảng & chart từ mock.
* [ ] `plan.contract.md` được chốt, khớp typescript types.

---

## M1 — Data Schema & Loader

**Goal:** Khai báo model, constraints, và loader JSON + validate mạnh tay.

### Owners & Tasks

* **Dev A:** Thiết kế `Course`, `Curriculum` (map id→Course, O(1) lookup).
* **Dev B:** `PlanConstraints` (max/min credits, numTerms, enforceCoreqTogether, offered\_terms…) + validate.
* **Dev C:** `Loader` đọc JSON → populate model + ném `std::runtime_error` kèm mã lỗi & ngữ cảnh; viết test valid/invalid.

### Folders & Files

```
/src/model/
├─ Course.h
├─ Curriculum.h
└─ PlanConstraints.h
/src/io/
├─ Loader.h
└─ Loader.cpp
/tests/
├─ loader_valid_test.cpp
└─ loader_invalid_test.cpp
/data/
├─ sample_small.json
├─ invalid_missing_field.json
├─ invalid_duplicate_id.json
└─ invalid_unknown_prereq.json
```

**Mục đích & Làm gì**

* **Course.h**: `id`, `name`, `credits`, `prereq[]`, `coreq[]`, optional `group`, `offered_terms[]`, `priority`.
* **Curriculum.h**: lưu `unordered_map<string, Course>`; API `get(id)`, `exists(id)`, `for_each`.
* **PlanConstraints.h**: default & validate phạm vi (min≤max, credits>0, numTerms>0).
* **Loader.cpp**: parse JSON, kiểm `credits>0`, id non-empty, không trùng id, prereq/coreq phải tồn tại, `offered_terms` ∈ \[1..numTerms].

### Checklist

* [ ] Parse dataset hợp lệ OK (`Loader_Valid_Sample_OK`).
* [ ] Bắt lỗi thiếu field, id trùng, prereq/coreq không tồn tại, offered\_terms out-of-range.
* [ ] ≥ 90% branch coverage cho Loader.

---

## M2 — Graph & TopoSort + Cycle Diagnosis

**Goal:** Dựng đồ thị, topo sort (Kahn), phát hiện vòng.

### Owners & Tasks

* **Dev A (owner):** `CourseGraph` (adj, indeg, idOf/idxOf), `TopoSort` (queue indeg=0), `CycleDiagnosis` (DFS/back-edge hoặc color state).
* **Dev B:** Review các edge cases (multi-sources, isolated, multi-sinks).
* **Dev C:** Chuẩn bị datasets DAG & cycle; viết test DAG + cycle.

### Folders & Files

```
/src/graph/
├─ CourseGraph.h
├─ CourseGraph.cpp
├─ TopoSort.h
├─ TopoSort.cpp
├─ CycleDiagnosis.h
└─ CycleDiagnosis.cpp
/tests/
└─ graph_topo_test.cpp
/data/
├─ branching.json
└─ cycle.json
```

**Mục đích & Làm gì**

* **CourseGraph**: build từ `Curriculum` + prereq edges; tính `indeg[]`.
* **TopoSort**: `TopoResult{bool success; vector<int> order;}`; fail nếu còn đỉnh `indeg>0`.
* **CycleDiagnosis**: trả về ≥1 chu trình (list id theo thứ tự) khi topo fail.

### Checklist

* [ ] Số đỉnh/cạnh, `indeg[]` đúng.
* [ ] DAG → `order.size()==V`.
* [ ] Cycle → `success=false` và liệt kê được chu trình hợp lệ.

---

## M3 — Longest Path (DAG) ⇒ Earliest Term

**Goal:** Tính `earliestTerm` (1‑based) dựa vào topo order.

### Owners & Tasks

* **Dev A:** Thuật toán longest path trên DAG: `level[v] = 1 + max(level[u])` với mọi `u→v`; nodes nguồn `level=1`.
* **Dev B:** Viết test: Branching\_OK, MultiSources\_OK, Disconnected\_OK.
* **Dev C:** Bổ sung dataset phù hợp & harness test.

### Folders & Files

```
/src/planner/
├─ LongestPathDag.h
└─ LongestPathDag.cpp
/tests/
└─ earliest_term_test.cpp
```

**Mục đích & Làm gì**

* **LongestPathDag**: dùng `TopoResult.order` để relax edges; trả `vector<int> earliestTermByIdx`.

### Checklist

* [ ] Kết quả `earliestTerm` đúng trên 3–4 case.
* [ ] Node cô lập → term=1 (hoặc theo policy đã chốt).

---

## M4 — TermAssigner v1 (Quota tín chỉ)

**Goal:** Gán môn vào kỳ theo quota, tôn trọng `earliestTerm`.

### Owners & Tasks

* **Dev B (owner):** `TermAssigner` greedy: duyệt theo `earliestTerm`; nếu vượt quota → đẩy sang kỳ sau; tie‑break theo out‑degree giảm dần.
* **Dev C:** CLI pipeline: `Loader → Graph → Topo → EarliestTerm → TermAssigner`; in bảng Term|Courses|Credits. Viết `assigner_quota_test.cpp` (3 case).
* **Dev A:** Review dependency correctness.

### Folders & Files

```
/src/planner/
├─ PlanTypes.h
├─ TermAssigner.h
└─ TermAssigner.cpp
/src/cli/
└─ main.cpp   (nhận -i input.json, in kết quả đơn giản)
/tests/
└─ assigner_quota_test.cpp
```

**Mục đích & Làm gì**

* **PlanTypes.h**: `Term{index, courseIds, credits}`, `PlanResult{feasible, terms, notes}`.
* **TermAssigner**: không để kỳ vượt `maxCredits`; giữ phụ thuộc.

### Checklist

* [ ] Không kỳ nào vượt quota.
* [ ] Phụ thuộc đúng (course chỉ xuất hiện sau khi prereq đã nằm ở kỳ trước).
* [ ] CLI in tổng số kỳ và tổng tín chỉ theo kỳ.

---

## M5 — Corequisite & Elective Groups

**Goal:** Gom nhóm coreq (cluster), chọn‑k elective groups.

### Owners & Tasks

* **Dev B (owner):** `Clusterizer` (connected components theo coreq); tổng credits cluster; detect infeasible nếu cluster>quota. `ElectiveResolver` chọn‑k theo `priority/credits` và loại courses không được chọn.
* **Dev A:** Kiểm tra đồ thị sau cluster (rút gọn nodes), đảm bảo cạnh giữa clusters đúng.
* **Dev C:** Data & tests cho coreq/electives; cập nhật CLI hiển thị badges (optional, nếu làm UI sớm).

### Folders & Files

```
/src/planner/
├─ Clusterizer.h
├─ Clusterizer.cpp
├─ ElectiveResolver.h
└─ ElectiveResolver.cpp
/tests/
└─ coreq_electives_test.cpp
/data/
├─ coreq_small.json
└─ elective_choose2.json
```

**Mục đích & Làm gì**

* **Clusterizer**: trả mapping `courseId → clusterId`, `clusterCredits[]`.
* **ElectiveResolver**: áp dụng policy chọn‑k, validate prereq sau khi loại bỏ.

### Checklist

* [ ] Cluster tạo đúng, không đứt quan hệ prereq.
* [ ] `clusterCredits` ≤ quota; nếu >quota → `feasible=false` + note.
* [ ] Elective chọn‑k hợp lệ; lỗi khi không đủ môn hợp lệ.

---

## M6 — Offered Terms & Infeasible Hints

**Goal:** Tôn trọng kỳ mở (`offered_terms`) và sinh gợi ý khi fail.

### Owners & Tasks

* **Dev B (owner):** `Hints` sinh đề xuất: tăng `numTerms`, tăng `maxCredits`, `relax_coreqTogether`, đổi elective.
* **Dev C:** CLI flags `--terms`, `--max-cred`, `--min-cred`, `--enforce-coreq`; in `notes` đẹp và có nhãn `Hint:`.
* **Dev A:** Review lọc theo `offered_terms` trong quá trình assign.

### Folders & Files

```
/src/planner/
├─ Hints.h
└─ Hints.cpp
/tests/
└─ offered_terms_hints_test.cpp
/data/
└─ offered_terms_wintersummer.json
```

**Mục đích & Làm gì**

* **Hints**: phân tích nguyên nhân infeasible (term đóng, quota, cluster) → sinh `notes` có cấu trúc, ví dụ: `{"increase_numTerms_to": 8}`.

### Checklist

* [ ] Course chỉ được gán vào kỳ có trong `offered_terms` (nếu có ràng buộc).
* [ ] Khi infeasible, `PlanResult.feasible=false` và có ≥1 note cụ thể, actionable.

---

## M7 — Writer (JSON/Markdown) & Explain

**Goal:** Xuất kết quả, giải thích vì sao một course nằm ở kỳ k.

### Owners & Tasks

* **Dev C (owner):** `Writer` xuất JSON & Markdown; CLI `--output plan.json|.md`.
* **Dev B:** `Explain`: tìm path dài nhất tới course (chain prereq) để giải thích.
* **Dev A:** Review performance & correctness.

### Folders & Files

```
/src/io/
├─ Writer.h
└─ Writer.cpp
/src/planner/
├─ Explain.h
└─ Explain.cpp
/tests/
└─ writer_explain_test.cpp
```

**Mục đích & Làm gì**

* **Writer**: JSON round‑trip (có thể đọc lại), Markdown gồm bảng Term | Courses | Credits + Notes.
* **Explain**: API `vector<string> whyPlaced(courseId)` trả chuỗi IDs theo thứ tự phụ thuộc.

### Checklist

* [ ] JSON xuất ra đọc lại được (round‑trip test).
* [ ] Markdown render đủ cột, dấu phân cách rõ.
* [ ] `Explain` trả đúng path phụ thuộc.

---

## M8 — Quality & Benchmark

**Goal:** Chất lượng, boundary cases, hiệu năng.

### Owners & Tasks

* **Dev A (owner):** Tạo dataset lớn & boundary (ZeroCourses, SingleCourse, LongChain\_100, StarShape).
* **Dev B:** Benchmark & tối ưu nếu >0.5s.
* **Dev C:** Optional `.clang-tidy` + hướng dẫn chạy benchmark.

### Folders & Files

```
/data/
└─ sample_medium.json   (60–80 courses, 300–600 edges)
/bench/
└─ README.md
/tests/
└─ boundary_cases_test.cpp
.optional: /.clang-tidy
```

**Mục đích & Làm gì**

* **bench/README.md**: cách đo thời gian chạy (`/usr/bin/time`, hoặc chrono).
* **boundary\_cases\_test.cpp**: xác nhận tính đúng ở rìa.

### Checklist

* [ ] Planner chạy < 0.5s trên CI với dataset trung bình.
* [ ] Toàn bộ boundary tests xanh.

---

## M9 — Visualization & What‑if (Optional)

**Goal:** Xuất Graphviz DOT và so sánh kịch bản what‑if.

### Owners & Tasks

* **Dev A (owner):** `DotExport` (tô màu theo term, label `id\n(credits)`).
* **Dev C:** `scripts/render_dot.sh` gọi `dot -Tpng` tạo ảnh nhanh; CLI thêm `--dot` để xuất file DOT.
* **Dev B:** Test 2 cấu hình what‑if (thay quota/terms) và so sánh số kỳ/variance.

### Folders & Files

```
/src/viz/
├─ DotExport.h
└─ DotExport.cpp
/scripts/
└─ render_dot.sh
/tests/
└─ viz_whatif_test.cpp
/data/
└─ graph_for_viz.json
```

**Mục đích & Làm gì**

* **DotExport**: API `string toDot(const PlanResult&)` hoặc trực tiếp từ Graph + term coloring.

### Checklist

* [ ] `dot -Tpng` không lỗi, tạo ảnh đúng.
* [ ] What‑if 2 cấu hình cho kết quả khác nhau có ý nghĩa.

---

## M10 — UI (Optional, Web/React)

**Goal:** UI mỏng hiển thị plan + biểu đồ, đọc JSON xuất từ CLI.

### Owners & Tasks

* **Dev C (owner):** App upload `curriculum.json` → call `planner_cli` (offline) hoặc upload `plan.json` → render; `PlanTable`, `CreditsBarChart`, `NotesPanel` states; UI cho `Explain` (modal path).
* **Dev B:** Mapping từ `PlanResult` sang props UI; đảm bảo `notes`, `explain` có cấu trúc ổn định.
* **Dev A:** Hỗ trợ hiển thị DOT (download PNG/SVG) nếu M9 đã làm.

### Folders & Files

```
/ui/
├─ README.md (run guide)
├─ src/pages/App.tsx
├─ src/components/PlanTable.tsx
├─ src/components/CreditsBarChart.tsx
├─ src/components/NotesPanel.tsx
└─ src/components/ExplainModal.tsx
```

**Mục đích & Làm gì**

* **App.tsx**: upload JSON → parse → render components; states cho feasible/infeasible.
* **ExplainModal**: click vào course → hiển thị chain phụ thuộc (nếu có `explain`).

### Checklist

* [ ] Load sample → render đúng bảng & chart.
* [ ] Infeasible → hiện `Notes` rõ ràng.
* [ ] Explain hoạt động cho >=1 course.

---

## Cross‑Milestone Checklists

* [ ] **Coding style** nhất quán; bật `-Wall -Wextra -Werror` (hoặc ít nhất cảnh báo cao).
* [ ] **Tests** chạy xanh trên CI ở mọi PR.
* [ ] **Datasets** có cả case hợp lệ & lỗi; mô tả ngắn trong `/data/README.md` (optional).
* [ ] **Docs**: Cập nhật `README.md` khi thêm flags/outputs mới.

---

## Nhánh Git gợi ý (không bắt buộc)

* `feat/m0-bootstrap-ci`
* `feat/m1-schema-loader`
* `feat/m2-graph-topo-cycle`
* `feat/m3-earliest-term`
* `feat/m4-term-assigner`
* `feat/m5-coreq-electives`
* `feat/m6-offered-hints`
* `feat/m7-writer-explain`
* `feat/m8-benchmark`
* `feat/m9-viz`
* `feat/m10-ui`

> Mỗi milestone: mở PR, checklist xanh, review chéo (A↔B↔C), rồi merge.

---

### Ghi chú

* **M0.5 UI Skeleton** là tùy chọn nếu muốn “nhìn thấy sản phẩm” sớm (hybrid approach). Không ảnh hưởng tiến độ core.
* Nếu thời gian gấp, ưu tiên hoàn thiện đến **M7 (Writer & Explain)** để có demo CLI + Markdown đầy đủ cho báo cáo.
