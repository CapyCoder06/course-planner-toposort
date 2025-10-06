#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "../io/Loader.h"
#include "../io/Writer.h"
#include "../graph/CourseGraph.h"
#include "../graph/TopoSort.h"
#include "../graph/CycleDiagnosis.h"
#include "../planner/LongestPathDag.h"
#include "../planner/TermAssigner.h"
#include "../planner/Hints.h"

using namespace std;
using namespace planner;

struct CliOptions
{
    string inputFile;
    string outputFile;
    int numTerms = -1;
    int maxCredits = -1;
    int minCredits = -1;
    bool enforceCoreq = true;
    bool hasEnforceCoreqOverride = false;
    bool verbose = false;
};

void printUsage(const char *progName)
{
    cout << "Usage: " << progName << " [OPTIONS]\n"
         << "Options:\n"
         << "  -i, --input <file>       Input JSON file (required)\n"
         << "  -o, --output <file>      Output file (.json or .md)\n"
         << "  --terms <N>              Override number of terms\n"
         << "  --max-cred <N>           Override max credits per term\n"
         << "  --min-cred <N>           Override min credits per term\n"
         << "  --enforce-coreq <bool>   Override coreq enforcement (true/false)\n"
         << "  -v, --verbose            Verbose output\n"
         << "  -h, --help               Show this help\n"
         << "\n"
         << "Output formats (auto-detected by extension):\n"
         << "  .json - JSON format (round-trip compatible)\n"
         << "  .md   - Markdown format (human-readable)\n";
}

CliOptions parseArgs(int argc, char *argv[])
{
    CliOptions opts;

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            printUsage(argv[0]);
            exit(0);
        }
        else if (arg == "-i" || arg == "--input")
        {
            if (i + 1 < argc)
                opts.inputFile = argv[++i];
        }
        else if (arg == "-o" || arg == "--output")
        {
            if (i + 1 < argc)
                opts.outputFile = argv[++i];
        }
        else if (arg == "--terms")
        {
            if (i + 1 < argc)
                opts.numTerms = stoi(argv[++i]);
        }
        else if (arg == "--max-cred")
        {
            if (i + 1 < argc)
                opts.maxCredits = stoi(argv[++i]);
        }
        else if (arg == "--min-cred")
        {
            if (i + 1 < argc)
                opts.minCredits = stoi(argv[++i]);
        }
        else if (arg == "--enforce-coreq")
        {
            if (i + 1 < argc)
            {
                string val = argv[++i];
                opts.enforceCoreq = (val == "true" || val == "1");
                opts.hasEnforceCoreqOverride = true;
            }
        }
        else if (arg == "-v" || arg == "--verbose")
        {
            opts.verbose = true;
        }
    }

    if (opts.inputFile.empty())
    {
        cerr << "Error: Input file required\n";
        printUsage(argv[0]);
        exit(1);
    }

    return opts;
}

void applyOverrides(PlanConstraints &constraints, const CliOptions &opts)
{
    if (opts.numTerms > 0)
    {
        cout << "Override: numTerms = " << opts.numTerms
             << " (was " << constraints.numTerms << ")\n";
        constraints.numTerms = opts.numTerms;
    }

    if (opts.maxCredits > 0)
    {
        cout << "Override: maxCreditsPerTerm = " << opts.maxCredits
             << " (was " << constraints.maxCreditsPerTerm << ")\n";
        constraints.maxCreditsPerTerm = opts.maxCredits;
    }

    if (opts.minCredits > 0)
    {
        cout << "Override: minCreditsPerTerm = " << opts.minCredits
             << " (was " << constraints.minCreditsPerTerm << ")\n";
        constraints.minCreditsPerTerm = opts.minCredits;
    }

    if (opts.hasEnforceCoreqOverride)
    {
        cout << "Override: enforceCoreqTogether = " << opts.enforceCoreq
             << " (was " << constraints.enforceCoreqTogether << ")\n";
        constraints.enforceCoreqTogether = opts.enforceCoreq;
    }
}

void printPlanSummary(const EnhancedPlanResult &result)
{
    cout << "\n========== PLAN SUMMARY ==========\n";
    cout << "Feasible: " << (result.feasible ? "YES" : "NO") << "\n";
    cout << "Total Terms Used: " << result.totalTermsUsed
         << " / " << result.constraints.numTerms << "\n";
    cout << "Total Credits: " << result.totalCredits << "\n\n";

    if (!result.terms.empty())
    {
        for (const auto &term : result.terms)
        {
            cout << "Term " << term.termNumber << " (" << term.totalCredits << " credits):\n";
            for (size_t i = 0; i < term.courseIds.size(); ++i)
            {
                cout << "  - " << term.courseIds[i] << " ("
                     << term.courseCredits[i] << " cr)\n";
            }
            cout << "\n";
        }
    }

    if (!result.warnings.empty())
    {
        cout << "========== WARNINGS ==========\n";
        for (const auto &warning : result.warnings)
        {
            cout << "⚠️  " << warning << "\n";
        }
        cout << "\n";
    }

    if (!result.notes.empty())
    {
        cout << "========== HINTS & NOTES ==========\n";
        for (size_t i = 0; i < result.notes.size(); ++i)
        {
            cout << "Hint " << (i + 1) << ": " << result.notes[i] << "\n";
        }
        cout << "\n";
    }
}

void printDetailedHints(const vector<HintNote> &hints)
{
    if (hints.empty())
        return;

    cout << "\n========== DETAILED RECOMMENDATIONS ==========\n";
    for (size_t i = 0; i < hints.size(); ++i)
    {
        cout << "\n[Hint " << (i + 1) << "]\n";
        cout << "Message: " << hints[i].message << "\n";

        if (!hints[i].actions.empty())
        {
            cout << "Suggested Actions:\n";
            for (const auto &[key, value] : hints[i].actions)
            {
                cout << "  --" << key << " " << value << "\n";
            }
        }
    }
    cout << "\n";
}

bool endsWith(const string &str, const string &suffix)
{
    if (str.length() < suffix.length())
        return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

int main(int argc, char *argv[])
{
    try
    {
        CliOptions opts = parseArgs(argc, argv);

        cout << "Loading curriculum from: " << opts.inputFile << "\n";
        auto loadResult = loadFromJsonFile(opts.inputFile);

        applyOverrides(loadResult.constraints, opts);

        if (loadResult.constraints.maxCreditsPerTerm < loadResult.constraints.minCreditsPerTerm)
        {
            throw runtime_error("Invalid: maxCreditsPerTerm < minCreditsPerTerm after overrides");
        }

        CourseGraph graph;
        graph.build(loadResult.curriculum);

        cout << "Courses loaded: " << graph.V << "\n";
        cout << "Building dependency graph...\n";

        auto topoResult = topoSort(graph);

        if (!topoResult.success)
        {
            cout << "\n[ERROR] Cyclic dependencies detected!\n";
            auto cycles = findOneCycle(graph);

            if (!cycles.empty())
            {
                cout << "Cycle found: ";
                for (size_t i = 0; i < cycles.size(); ++i)
                {
                    cout << graph.idxToId.at(cycles[i]);
                    if (i < cycles.size() - 1)
                        cout << " -> ";
                }
                cout << "\n";
            }

            return 1;
        }

        cout << "Topological sort: OK\n";

        auto earliestTerms = computeEarliestTerms(graph, topoResult);
        cout << "Earliest terms computed\n";

        vector<int> creditsByIdx(graph.V);
        loadResult.curriculum.for_each([&](const Course &c)
                                       {
            int idx = graph.idToIdx.at(c.id);
            creditsByIdx[idx] = c.credits; });

        cout << "Assigning courses to terms...\n";
        auto planResult = assignTermsGreedy(
            graph,
            topoResult,
            earliestTerms.termByIdx,
            creditsByIdx,
            loadResult.constraints);

        auto enhancedResult = Writer::enhance(
            planResult,
            graph,
            loadResult.curriculum,
            loadResult.constraints);

        printPlanSummary(enhancedResult);

        if (!planResult.ok)
        {
            auto hints = Hints::analyze(
                loadResult.constraints.numTerms,
                loadResult.constraints.maxCreditsPerTerm,
                loadResult.constraints.enforceCoreqTogether,
                false,
                false);

            printDetailedHints(hints);
        }

        if (!opts.outputFile.empty())
        {
            cout << "Writing output to: " << opts.outputFile << "\n";

            if (endsWith(opts.outputFile, ".json"))
            {
                Writer::writeJson(enhancedResult, opts.outputFile);
                cout << "JSON output written successfully\n";
            }
            else if (endsWith(opts.outputFile, ".md"))
            {
                Writer::writeMarkdown(enhancedResult, opts.outputFile);
                cout << "Markdown output written successfully\n";
            }
            else
            {
                cerr << "Warning: Unknown output format. Use .json or .md extension.\n";
                cerr << "Defaulting to JSON format.\n";
                Writer::writeJson(enhancedResult, opts.outputFile + ".json");
            }
        }

        return planResult.ok ? 0 : 1;
    }
    catch (const LoadException &e)
    {
        cerr << "Load Error [" << e.getErrorCode() << "]: "
             << e.what() << "\n";
        if (!e.getContext().empty())
        {
            cerr << "Context: " << e.getContext() << "\n";
        }
        return 2;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}