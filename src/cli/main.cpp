#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WFileUpload.h>
#include <Wt/WProgressBar.h>
#include <Wt/WBreak.h>
#include <Wt/WTable.h>
#include <Wt/WBorderLayout.h>
#include <Wt/WScrollArea.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Parser.h>

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>

using namespace Wt;

struct Course {
  std::string id, name;
  int credits{};
  std::vector<std::string> prereq;
};

class PlannerApp : public WApplication {
public:
  PlannerApp(const WEnvironment& env) : WApplication(env) {
    setTitle("Course Planner (Wt6 demo)");
    root()->setPadding(12);

    auto title = root()->addWidget(std::make_unique<WText>("<h2>Course Planner (Wt6)</h2>"));
    title->setTextFormat(TextFormat::XHTML);
    root()->addWidget(std::make_unique<WText>(
        "Upload curriculum JSON → tính topo (Kahn), phát hiện cycle, xếp kỳ theo earliest-term."));
    root()->addWidget(std::make_unique<WBreak>());

    // Upload UI
    auto uploadBox = root()->addWidget(std::make_unique<WContainerWidget>());
    auto fu = uploadBox->addWidget(std::make_unique<WFileUpload>());
    fu->setFileTextSize(64 * 1024 * 1024);
    fu->setMultiple(false);
    fu->setProgressBar(std::make_unique<WProgressBar>());
    fu->changed().connect(fu, &WFileUpload::upload);

    _status = uploadBox->addWidget(std::make_unique<WText>("<p>Chọn file JSON…</p>"));
    _status->setTextFormat(TextFormat::XHTML);

    fu->uploaded().connect([=] {
      try {
        auto path = fu->spoolFileName(); // file tạm
        std::ifstream in(path, std::ios::binary);
        std::stringstream ss; ss << in.rdbuf();
        auto jsonStr = ss.str();

        Wt::Json::Value rootV;
        if (!Wt::Json::parse(jsonStr, rootV)) {
          _status->setText("<p style='color:#b91c1c'>JSON parse lỗi.</p>");
          return;
        }
        if (!rootV.isObject()) {
          _status->setText("<p style='color:#b91c1c'>Root phải là object.</p>");
          return;
        }
        const auto& obj = rootV.asObject();
        if (!obj.contains("courses") || !obj.get("courses").isArray()) {
          _status->setText("<p style='color:#b91c1c'>Thiếu mảng courses[].</p>");
          return;
        }

        // Đọc courses
        std::vector<Course> courses;
        std::unordered_map<std::string,int> id2idx;
        const auto& arr = obj.get("courses").asArray();
        courses.reserve(arr.size());
        for (const auto& v : arr) {
          if (!v.isObject()) continue;
          const auto& co = v.asObject();
          Course c;
          if (co.contains("id")) c.id = co.get("id").toString().toUTF8();
          if (co.contains("name")) c.name = co.get("name").toString().toUTF8();
          if (co.contains("credits")) c.credits = co.get("credits").toNumber();
          if (co.contains("prerequisites") && co.get("prerequisites").isArray()) {
            for (const auto& pv : co.get("prerequisites").asArray()) {
              c.prereq.push_back(pv.toString().toUTF8());
            }
          }
          if (c.id.empty() || c.credits <= 0) {
            _status->setText("<p style='color:#b91c1c'>Mỗi course cần id (string) và credits > 0.</p>");
            return;
          }
          id2idx[c.id] = (int)courses.size();
          courses.push_back(std::move(c));
        }

        // Xây đồ thị
        int n = (int)courses.size();
        std::vector<std::vector<int>> adj(n);
        std::vector<int> indeg(n, 0);
        for (int i = 0; i < n; ++i) {
          for (auto& pid : courses[i].prereq) {
            auto it = id2idx.find(pid);
            if (it == id2idx.end()) {
              _status->setText("<p style='color:#b91c1c'>Prerequisite không tồn tại: " + pid + "</p>");
              return;
            }
            int u = it->second, v = i;
            if (u == v) {
              _status->setText("<p style='color:#b91c1c'>Self-loop tại " + courses[i].id + "</p>");
              return;
            }
            adj[u].push_back(v);
            indeg[v]++;
          }
        }

        // Kahn topo
        std::queue<int> q;
        for (int i = 0; i < n; ++i) if (indeg[i] == 0) q.push(i);
        std::vector<int> topo;
        std::vector<int> indeg2 = indeg;
        while (!q.empty()) {
          int u = q.front(); q.pop();
          topo.push_back(u);
          for (int v : adj[u]) if (--indeg2[v] == 0) q.push(v);
        }
        if ((int)topo.size() != n) {
          _status->setText("<p style='color:#b91c1c'>Cycle detected (đồ thị không phải DAG).</p>");
          showCoursesBrief(courses);
          return;
        }

        // Earliest-term (level)
        std::vector<int> term(n, 1);
        for (int u : topo) {
          for (int v : adj[u]) term[v] = std::max(term[v], term[u] + 1);
        }
        int maxTerm = 1;
        for (int t : term) maxTerm = std::max(maxTerm, t);

        _status->setText("<p style='color:#166534'>OK – Đã lập kế hoạch theo earliest-term.</p>");

        // Render bảng kết quả theo kỳ
        renderPlanTable(courses, term, maxTerm);

      } catch (const std::exception& e) {
        _status->setText(std::string("<p style='color:#b91c1c'>Lỗi: ") + e.what() + "</p>");
      }
    });
  }

private:
  WText* _status{};
  WContainerWidget* _planArea{};

  void showCoursesBrief(const std::vector<Course>& courses) {
    if (!_planArea) _planArea = root()->addWidget(std::make_unique<WContainerWidget>());
    _planArea->clear();
    auto tbl = _planArea->addWidget(std::make_unique<WTable>());
    tbl->setHeaderCount(1);
    tbl->elementAt(0,0)->addWidget(std::make_unique<WText>("<b>ID</b>"))->setTextFormat(TextFormat::XHTML);
    tbl->elementAt(0,1)->addWidget(std::make_unique<WText>("<b>Name</b>"))->setTextFormat(TextFormat::XHTML);
    tbl->elementAt(0,2)->addWidget(std::make_unique<WText>("<b>Credits</b>"))->setTextFormat(TextFormat::XHTML);

    int r = 1;
    for (auto& c : courses) {
      tbl->elementAt(r,0)->addWidget(std::make_unique<WText>(c.id));
      tbl->elementAt(r,1)->addWidget(std::make_unique<WText>(c.name));
      tbl->elementAt(r,2)->addWidget(std::make_unique<WText>(std::to_string(c.credits)));
      r++;
    }
  }

  void renderPlanTable(const std::vector<Course>& courses,
                       const std::vector<int>& term, int maxTerm) {
    if (!_planArea) _planArea = root()->addWidget(std::make_unique<WContainerWidget>());
    _planArea->clear();

    auto wrap = _planArea->addWidget(std::make_unique<WContainerWidget>());
    wrap->setOverflow(Overflow::Auto);
    auto tbl = wrap->addWidget(std::make_unique<WTable>());
    tbl->setHeaderCount(1);

    // Header: Term 1..maxTerm
    tbl->elementAt(0,0)->addWidget(std::make_unique<WText>("<b>Term</b>"))->setTextFormat(TextFormat::XHTML);
    tbl->elementAt(0,1)->addWidget(std::make_unique<WText>("<b>Courses</b>"))->setTextFormat(TextFormat::XHTML);
    tbl->elementAt(0,2)->addWidget(std::make_unique<WText>("<b>Total Credits</b>"))->setTextFormat(TextFormat::XHTML);

    for (int t = 1, row = 1; t <= maxTerm; ++t, ++row) {
      std::vector<int> idxs;
      int sum = 0;
      for (int i = 0; i < (int)courses.size(); ++i) if (term[i] == t) {
        idxs.push_back(i); sum += courses[i].credits;
      }
      tbl->elementAt(row,0)->addWidget(std::make_unique<WText>("Term " + std::to_string(t)));

      std::stringstream list;
      for (size_t k = 0; k < idxs.size(); ++k) {
        const auto& c = courses[idxs[k]];
        if (k) list << " | ";
        list << c.id << " (" << c.credits << "cr) - " << c.name;
      }
      tbl->elementAt(row,1)->addWidget(std::make_unique<WText>(list.str()));
      tbl->elementAt(row,2)->addWidget(std::make_unique<WText>(std::to_string(sum)));
    }
  }
};

std::unique_ptr<WApplication> createApp(const WEnvironment& env) {
  return std::make_unique<PlannerApp>(env);
}

int main(int argc, char** argv) {
  return WRun(argc, argv, &createApp);
}
