/*Parse tham số lệnh cơ bản: validate, build, explain, filepath, flags (terms, max/min credits, enforceCoreq).

Gọi Loader → Graph → (Topo/LP/Assigner) → in kết quả (tạm: tóm tắt).

Xử lý lỗi & exit code phù hợp.

AC: Chạy được luồng validate và build cơ bản trên data/sample_small.json.*/
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "../io/Loader.h"
#include "../graph/CourseGraph.h"
#include "../graph/TopoSort.h"
#include "../graph/CycleDiagnosis.h"
#include "../planner/LongestPathDag.h"
#include "../planner/TermAssigner.h"
#include "../model/PlanConstraints.h"

using namespace std;
using namespace planner;

struct CliArgs
{
    string command;
    string filepath;
    int numTerms = -1;
    int maxCredits = -1;
    int minCredits = -1;
    bool enforceCoreq = false;
    bool help = false;
};

void printHelp()
{
    cout << "Usage: planner <command> <filepath> [options]\n\n";
    cout << "Commands:\n";
    cout << "  validate <file>     Validate curriculum JSON file\n";
    cout << "  build <file>        Build course schedule plan\n";
    cout << "  explain <file>      Explain course dependencies\n\n";
    cout << "Options:\n";
    cout << "  --terms <n>         Number of terms (overrides JSON)\n";
    cout << "  --max-credits <n>   Max credits per term (overrides JSON)\n";
    cout << "  --min-credits <n>   Min credits per term (overrides JSON)\n";
    cout << "  --enforce-coreq     Enforce corequisites in same term\n";
    cout << "  --help              Show this help message\n\n";
    cout << "Examples:\n";
    cout << "  planner validate data/sample.json\n";
    cout << "  planner build data/sample.json --terms 8 --max-credits 18\n";
    cout << "  planner explain data/sample.json\n";
}

CliArgs parseArgs(int argc, char *argv[])
{
    CliArgs args;

    if (argc < 2)
    {
        args.help = true;
        return args;
    }

    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];

        if (arg == "--help" || arg == "-h")
        {
            args.help = true;
            return args;
        }
        else if (arg == "--terms" && i + 1 < argc)
        {
            args.numTerms = stoi(argv[++i]);
        }
        else if (arg == "--max-credits" && i + 1 < argc)
        {
            args.maxCredits = stoi(argv[++i]);
        }
        else if (arg == "--min-credits" && i + 1 < argc)
        {
            args.minCredits = stoi(argv[++i]);
        }
        else if (arg == "--enforce-coreq")
        {
            args.enforceCoreq = true;
        }
        else if (args.command.empty())
        {
            args.command = arg;
        }
        else if (args.filepath.empty())
        {
            args.filepath = arg;
        }
    }

    return args;
}

int validateCommand(const CliArgs &args)
{
    try
    {
        cout << "Validating: " << args.filepath << "\n";

        auto result = loadFromJsonFile(args.filepath);

        cout << "JSON parsing successful\n";
        cout << "Constraints validation passed\n";

        int courseCount = 0;
        result.curriculum.for_each([&](const Course &)
                                   { courseCount++; });

        cout << "Found " << courseCount << " courses\n";

        CourseGraph graph;
        graph.build(result.curriculum);

        cout << "Course graph built (" << graph.V << " nodes)\n";

        auto topoResult = topoSort(graph);
        if (!topoResult.success)
        {
            auto cycle = findOneCycle(graph);
            cout << "Cycle detected in prerequisites:\n";
            for (int idx : cycle)
            {
                cout << "  → " << graph.idxToId[idx] << "\n";
            }
            return 1;
        }

        cout << "No cycles detected\n";
        cout << "All validations passed\n";
        return 0;
    }
    catch (const LoadException &e)
    {
        cerr << "Load Error [" << e.getErrorCode() << "] at " << e.getContext() << ":\n";
        cerr << "  " << e.what() << "\n";
        return 1;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int buildCommand(const CliArgs &args)
{
    try
    {
        cout << "Building plan from: " << args.filepath << "\n\n";

        auto result = loadFromJsonFile(args.filepath);

        if (args.numTerms > 0)
        {
            result.constraints.numTerms = args.numTerms;
        }
        if (args.maxCredits > 0)
        {
            result.constraints.maxcredits = args.maxCredits;
        }
        if (args.minCredits > 0)
        {
            result.constraints.mincredits = args.minCredits;
        }
        if (args.enforceCoreq)
        {
            result.constraints.enforceCoreqTogether = true;
        }

        result.constraints.validate();

        cout << "Configuration:\n";
        cout << "  Terms: " << result.constraints.numTerms << "\n";
        cout << "  Credits per term: " << result.constraints.mincredits
             << " - " << result.constraints.maxcredits << "\n";
        cout << "  Enforce corequisites: "
             << (result.constraints.enforceCoreqTogether ? "Yes" : "No") << "\n\n";

        CourseGraph graph;
        graph.build(result.curriculum);

        auto topoResult = topoSort(graph);
        if (!topoResult.success)
        {
            auto cycle = findOneCycle(graph);
            cerr << "Error: Cycle detected in prerequisites:\n";
            for (int idx : cycle)
            {
                cerr << "  → " << graph.idxToId[idx] << "\n";
            }
            return 1;
        }

        cout << "Topological sort: OK (" << topoResult.order.size() << " courses)\n";

        auto earliestTerms = computeEarliestTerms(graph, topoResult);

        cout << "Earliest terms computed\n";

        vector<int> creditsByIdx(graph.V);
        result.curriculum.for_each([&](const Course &c)
                                   {
            int idx = graph.idToIdx.at(c.id);
            creditsByIdx[idx] = c.credits; });

        auto plan = assignTermsGreedy(graph, topoResult, earliestTerms.termByIdx,
                                      creditsByIdx, result.constraints);

        if (!plan.ok)
        {
            cerr << "\nError: Could not create feasible plan\n";
            for (const auto &note : plan.notes)
            {
                cerr << "  " << note << "\n";
            }
            return 1;
        }

        cout << "\n--- COURSE SCHEDULE ---n\n";

        map<int, vector<string>> termCourses;
        map<int, int> termCredits;

        for (int idx = 0; idx < graph.V; idx++)
        {
            int term = plan.termOfIdx[idx];
            if (term > 0)
            {
                termCourses[term].push_back(graph.idxToId[idx]);
                termCredits[term] += creditsByIdx[idx];
            }
        }

        int totalCredits = 0;
        for (const auto &[term, courses] : termCourses)
        {
            cout << "Term " << term << " (" << termCredits[term] << " credits):\n";
            for (const string &courseId : courses)
            {
                const Course &course = result.curriculum.get(courseId);
                cout << "  " << course.id << " - " << course.name
                     << " (" << course.credits << " cr)\n";
            }
            totalCredits += termCredits[term];
            cout << "\n";
        }

        cout << "Total: " << termCourses.size() << " terms, "
             << totalCredits << " credits\n";

        if (!plan.notes.empty())
        {
            cout << "\nNotes:\n";
            for (const auto &note : plan.notes)
            {
                cout << "  • " << note << "\n";
            }
        }

        return 0;
    }
    catch (const LoadException &e)
    {
        cerr << "Load Error [" << e.getErrorCode() << "] at " << e.getContext() << ":\n";
        cerr << "  " << e.what() << "\n";
        return 1;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int explainCommand(const CliArgs &args)
{
    try
    {
        cout << "Explaining dependencies in: " << args.filepath << "\n\n";

        auto result = loadFromJsonFile(args.filepath);

        CourseGraph graph;
        graph.build(result.curriculum);

        auto topoResult = topoSort(graph);
        if (!topoResult.success)
        {
            auto cycle = findOneCycle(graph);
            cerr << "Error: Cycle detected:\n";
            for (int idx : cycle)
            {
                cerr << "  → " << graph.idxToId[idx] << "\n";
            }
            return 1;
        }

        auto earliestTerms = computeEarliestTerms(graph, topoResult);

        cout << "=== COURSE DEPENDENCIES ===\n\n";

        vector<pair<int, string>> coursesByTerm;
        for (int idx = 0; idx < graph.V; idx++)
        {
            coursesByTerm.push_back({earliestTerms.termByIdx[idx], graph.idxToId[idx]});
        }
        sort(coursesByTerm.begin(), coursesByTerm.end());

        for (const auto &[term, courseId] : coursesByTerm)
        {
            const Course &course = result.curriculum.get(courseId);

            cout << course.id << " - " << course.name << "\n";
            cout << "  Earliest term: " << term << "\n";
            cout << "  Credits: " << course.credits << "\n";

            if (!course.prerequisite.empty())
            {
                cout << "  Prerequisites:\n";
                for (const string &prereqId : course.prerequisite)
                {
                    int prereqIdx = graph.idToIdx.at(prereqId);
                    int prereqTerm = earliestTerms.termByIdx[prereqIdx];
                    cout << "    → " << prereqId << " (term " << prereqTerm << ")\n";
                }
            }

            if (!course.corequisite.empty())
            {
                cout << "  Corequisites:\n";
                for (const string &coreqId : course.corequisite)
                {
                    cout << "    ⊗ " << coreqId << "\n";
                }
            }

            if (!graph.adj[graph.idToIdx.at(courseId)].empty())
            {
                cout << "  Required by:\n";
                for (int depIdx : graph.adj[graph.idToIdx.at(courseId)])
                {
                    cout << "    ← " << graph.idxToId[depIdx] << "\n";
                }
            }

            cout << "\n";
        }

        cout << "=== SUMMARY ===\n";
        cout << "Total courses: " << graph.V << "\n";
        cout << "Maximum depth: " << *max_element(earliestTerms.termByIdx.begin(), earliestTerms.termByIdx.end()) << " terms\n";

        int sources = 0, sinks = 0;
        for (int i = 0; i < graph.V; i++)
        {
            if (graph.indeg[i] == 0)
                sources++;
            if (graph.adj[i].empty())
                sinks++;
        }
        cout << "Source courses (no prerequisites): " << sources << "\n";
        cout << "Sink courses (not required by others): " << sinks << "\n";

        return 0;
    }
    catch (const LoadException &e)
    {
        cerr << "Load Error [" << e.getErrorCode() << "] at " << e.getContext() << ":\n";
        cerr << "  " << e.what() << "\n";
        return 1;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int main(int argc, char *argv[])
{
    CliArgs args = parseArgs(argc, argv);

    if (args.help || args.command.empty())
    {
        printHelp();
        return args.help ? 0 : 1;
    }

    if (args.filepath.empty())
    {
        cerr << "Error: No input file specified\n";
        cerr << "Use --help for usage information\n";
        return 1;
    }

    if (args.command == "validate")
    {
        return validateCommand(args);
    }
    else if (args.command == "build")
    {
        return buildCommand(args);
    }
    else if (args.command == "explain")
    {
        return explainCommand(args);
    }
    else
    {
        cerr << "Error: Unknown command '" << args.command << "'\n";
        cerr << "Valid commands: validate, build, explain\n";
        return 1;
    }
}