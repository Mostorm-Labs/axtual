#pragma once

#include "axtual/binding.hpp"
#include "axtual/endpoint.hpp"
#include "axtual/enhancement.hpp"
#include "axtual/policy.hpp"
#include "axtual/snapshot.hpp"

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace i30_02 {

struct TestCase {
    std::string_view name;
    std::function<void()> run;
};

inline void require(bool value, std::string_view message) {
    if (!value) throw std::runtime_error(std::string(message));
}

inline void makeSupported(axtual::Endpoint& endpoint) {
    endpoint.capability = axtual::Capability::kSupported;
    endpoint.available = true;
}

void appendEndpointCases(std::vector<TestCase>& tests);
void appendEnhancementCases(std::vector<TestCase>& tests);
void appendRepairCases(std::vector<TestCase>& tests);
std::vector<TestCase> cases();

}  // namespace i30_02
