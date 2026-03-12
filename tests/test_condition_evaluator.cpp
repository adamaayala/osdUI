#include <catch2/catch_test_macros.hpp>
#include "script/condition_evaluator.hpp"
#include "mocks/mock_variable_store.hpp"

using namespace osdui;
using namespace osdui::test;

TEST_CASE("ConditionEvaluator: empty expression returns true") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    REQUIRE(eval.evaluate(L"", vars) == true);
}

TEST_CASE("ConditionEvaluator: literal true/false") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    REQUIRE(eval.evaluate(L"true",  vars) == true);
    REQUIRE(eval.evaluate(L"false", vars) == false);
    REQUIRE(eval.evaluate(L"1",     vars) == true);
    REQUIRE(eval.evaluate(L"0",     vars) == false);
}

TEST_CASE("ConditionEvaluator: variable substitution + equality") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    vars.set(L"OSDModel", L"HP EliteBook");
    REQUIRE(eval.evaluate(L"%OSDModel% == \"HP EliteBook\"", vars) == true);
    REQUIRE(eval.evaluate(L"%OSDModel% == \"Dell XPS\"",     vars) == false);
}

TEST_CASE("ConditionEvaluator: variable substitution + not-equals") {
    script::ConditionEvaluator eval;
    MapVariableStore vars;
    vars.set(L"OSDIsVM", L"false");
    REQUIRE(eval.evaluate(L"%OSDIsVM% != \"true\"", vars) == true);
}
