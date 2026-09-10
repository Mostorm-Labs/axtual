#include "i30_02_test.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace i30_02 {

std::vector<TestCase> cases() {
    std::vector<TestCase> tests;
    tests.reserve(35);
    appendEndpointCases(tests);
    appendEnhancementCases(tests);
    appendRepairCases(tests);
    return tests;
}

std::string environmentIdentity() {
#if defined(_WIN32)
    char* runner = nullptr;
    std::size_t runnerLength = 0;
    if (_dupenv_s(&runner, &runnerLength, "RUNNER_OS") == 0 && runner != nullptr) {
        std::string result(runner, runnerLength > 0 ? runnerLength - 1 : 0);
        std::free(runner);
        return result + ":github-actions";
    }
    return "Windows:developer-host";
#else
    if (const char* runner = std::getenv("RUNNER_OS"); runner != nullptr) {
        return std::string(runner) + ":github-actions";
    }
#if defined(__APPLE__)
    return "macOS:developer-host";
#elif defined(__linux__)
    return "Linux:developer-host";
#else
    return "Unknown:developer-host";
#endif
#endif
}

std::string oracleFor(std::string_view name) {
    if (name.find("policy") != std::string_view::npos ||
        name.find("recommended") != std::string_view::npos ||
        name.find("hidden") != std::string_view::npos ||
        name.find("caller") != std::string_view::npos) {
        return name.find("selection") != std::string_view::npos ||
                       name.find("caller") != std::string_view::npos
                   ? "O-POLICY-ENFORCE"
                   : "O-POLICY-TRUTH";
    }
    if (name.find("enhancement") != std::string_view::npos) return "O-ENH-INDEP";
    if (name.find("loop") != std::string_view::npos || name.find("virtual") != std::string_view::npos)
        return "O-SELFLOOP";
    if (name.find("reconnect") != std::string_view::npos || name.find("gone") != std::string_view::npos)
        return "O-IDENTITY";
    if (name.find("demand") != std::string_view::npos || name.find("inventory") != std::string_view::npos)
        return "O-IDLE";
    return "O-POLICY-TRUTH";
}

}  // namespace i30_02

int main(int argc, char** argv) {
    std::string selected = argc == 3 && std::string_view(argv[1]) == "--case" ? argv[2] : "";
    const auto allCases = i30_02::cases();

    if (argc == 3 && std::string_view(argv[1]) == "--evidence") {
        std::ofstream evidence(argv[2]);
        if (!evidence) return 3;
        evidence << "{\n"
                 << "  \"verification_spec\": \"P20-v0.3\",\n"
                 << "  \"package_id\": \"AXTUAL-I30-02-P31-v0.1\",\n"
                 << "  \"slice\": \"I30-02\",\n"
                 << "  \"evidence_family\": \"EA-B01\",\n"
                 << "  \"claim_scope\": \"common_semantic_contribution_only\",\n"
                 << "  \"obligations\": [\"VO-END-02\", \"VO-END-03\", \"VO-END-04\", \"VO-END-06\"],\n"
                 << "  \"authority_refs\": [\"P02-v0.3\", \"P03-v0.3\", \"P14-v0.4\", \"P15-v0.4\", \"P16-v0.4\", \"P17-v0.5\", \"P18-v0.5\", \"P20-v0.3\", \"P30-v0.3\"],\n"
                 << "  \"task_anchor\": \"" << AXTUAL_TASK_ANCHOR << "\",\n"
                 << "  \"build_identity\": \"" << AXTUAL_BUILD_IDENTITY << "\",\n"
                 << "  \"platform_environment\": \"" << i30_02::environmentIdentity() << "\",\n"
                 << "  \"execution_method\": \"I30-02-deterministic-semantic-conformance\",\n"
                 << "  \"oracle\": [\"O-POLICY-TRUTH\", \"O-POLICY-ENFORCE\", \"O-ENH-INDEP\", \"O-SELFLOOP\", \"O-IDENTITY\", \"O-IDLE\"],\n"
                 << "  \"case_results\": [\n";
        bool allPassed = true;
        for (std::size_t index = 0; index < allCases.size(); ++index) {
            std::string result = "PASS";
            try {
                allCases[index].run();
            } catch (const std::exception&) {
                result = "FAIL";
                allPassed = false;
            }
            evidence << "    {\"name\": \"" << allCases[index].name << "\", \"oracle\": \""
                     << i30_02::oracleFor(allCases[index].name) << "\", \"result\": \"" << result << "\"}"
                     << (index + 1 == allCases.size() ? "\n" : ",\n");
        }
        evidence << "  ],\n  \"result\": \"" << (allPassed ? "PASS" : "FAIL") << "\"\n}\n";
        return evidence.good() && allPassed ? 0 : 1;
    }

    bool ran = false;
    bool allPassed = true;
    for (const auto& test : allCases) {
        if (!selected.empty() && test.name != selected) continue;
        ran = true;
        try {
            test.run();
            std::cout << "PASS " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "FAIL " << test.name << ": " << error.what() << '\n';
            allPassed = false;
        }
        if (!selected.empty()) break;
    }
    return ran ? (allPassed ? 0 : 1) : 2;
}
