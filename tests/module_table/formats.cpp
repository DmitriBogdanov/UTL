#include "tests/common.hpp"

#include "include/UTL/table.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

const auto format_number = [](double x) { return table::numeric{x, std::chars_format::scientific, 5}; };

TEST_CASE("Formats / ASCII") {

    table::ascii table(6);

    table.hline();
    table.cell("Task", "Priority", "Time", "Weight", "Coef", "Completion");
    table.hline();
    table.cell("Work 1", 10, 1.35, 0.237f, format_number(1.137), true);
    table.cell("Work 2", 20, 2.45, 0.145f, format_number(4.654), false);
    table.hline();

    const std::string result = table.format();

    const std::string reference = R"(|--------|----------|------|--------|-------------|------------|)"
                                  "\n"
                                  R"(| Task   | Priority | Time | Weight | Coef        | Completion |)"
                                  "\n"
                                  R"(|--------|----------|------|--------|-------------|------------|)"
                                  "\n"
                                  R"(| Work 1 | 10       | 1.35 | 0.237  | 1.13700e+00 | true       |)"
                                  "\n"
                                  R"(| Work 2 | 20       | 2.45 | 0.145  | 4.65400e+00 | false      |)"
                                  "\n"
                                  R"(|--------|----------|------|--------|-------------|------------|)"
                                  "\n";

    CHECK(result == reference);
}

TEST_CASE("Formats / Markdown") {

    table::markdown table({"Task", "Priority", "Time", "Weight", "Coef", "Completion"});

    table.cell("Work 1", 10, 1.35, 0.237f, format_number(1.137), true);
    table.cell("Work 2", 20, 2.45, 0.145f, format_number(4.654), false);

    const std::string result = table.format();

    const std::string reference = R"(| Task   | Priority | Time | Weight | Coef        | Completion |)"
                                  "\n"
                                  R"(| ------ | -------- | ---- | ------ | ----------- | ---------- |)"
                                  "\n"
                                  R"(| Work 1 | 10       | 1.35 | 0.237  | 1.13700e+00 | `true`     |)"
                                  "\n"
                                  R"(| Work 2 | 20       | 2.45 | 0.145  | 4.65400e+00 | `false`    |)"
                                  "\n";

    CHECK(result == reference);
}

TEST_CASE("Formats / LaTeX") {

    table::latex table(6);

    table.hline();
    table.cell("Task", "Priority", "Time", "Weight", "Coef", "Completion");
    table.hline();
    table.cell("Work 1", 10, 1.35, 0.237f, format_number(1.137), true);
    table.cell("Work 2", 20, 2.45, 0.145f, format_number(4.654), false);
    table.cell("Work 2", 20, 2.45, 0.145f, format_number(10000), false);
    table.hline();

    const std::string result = table.format();

    const std::string reference = R"(\begin{tabular}{|c|c|c|c|c|c|})"
                                  "\n"
                                  R"(\hline)"
                                  "\n"
                                  R"(    Task   & Priority & Time   & Weight  & Coef                   & Completion \\)"
                                  "\n"
                                  R"(\hline)"
                                  "\n"
                                  R"(    Work 1 & $10$     & $1.35$ & $0.237$ & $1.13700 \cdot 10^{0}$ & true       \\)"
                                  "\n"
                                  R"(    Work 2 & $20$     & $2.45$ & $0.145$ & $4.65400 \cdot 10^{0}$ & false      \\)"
                                  "\n"
                                  R"(    Work 2 & $20$     & $2.45$ & $0.145$ & $10^{4}$               & false      \\)"
                                  "\n"
                                  R"(\hline)"
                                  "\n"
                                  R"(\end{tabular})"
                                  "\n";

    CHECK(result == reference);
}

TEST_CASE("Formats / Mathematica") {

    table::mathematica table(6);

    table.hline();
    table.cell("Task", "Priority", "Time", "Weight", "Coef", "Completion");
    table.hline();
    table.cell("Work 1", 10, 1.35, 0.237f, format_number(1.137), true);
    table.cell("Work 2", 20, 2.45, 0.145f, format_number(4.654), false);
    table.cell("Work 2", 20, 2.45, 0.145f, format_number(10000), false);
    table.hline();

    const std::string result = table.format();

    const std::string reference = R"(Grid[{)"
                                  "\n"
                                  R"(    { "Task"  , "Priority", "Time", "Weight", "Coef"      , "Completion" },)"
                                  "\n"
                                  R"(    { "Work 1", 10        , 1.35  , 0.237   , 1.13700*^+00, True         },)"
                                  "\n"
                                  R"(    { "Work 2", 20        , 2.45  , 0.145   , 4.65400*^+00, False        },)"
                                  "\n"
                                  R"(    { "Work 2", 20        , 2.45  , 0.145   , 1.00000*^+04, False        })"
                                  "\n"
                                  R"(}, Dividers -> {All, {True, True, False, False, True}}])"
                                  "\n";

    CHECK(result == reference);
}

TEST_CASE("Formats / Mathematica") {

    table::csv table(6);

    table.cell("Task", "Priority", "Time", "Weight", "Coef", "Completion");
    table.cell("Work 1", 10, 1.35, 0.237f, format_number(1.137), true);
    table.cell("Work 2", 20, 2.45, 0.145f, format_number(4.654), false);

    const std::string result = table.format();

    const std::string reference = R"("Task","Priority","Time","Weight","Coef","Completion")"
                                  "\n"
                                  R"("Work 1",10,1.35,0.237,1.13700e+00,true)"
                                  "\n"
                                  R"("Work 2",20,2.45,0.145,4.65400e+00,false)"
                                  "\n";

    CHECK(result == reference);
}