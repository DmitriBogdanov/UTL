#include "benchmarks/common.hpp"

#include "include/UTL/json.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp"

// Libraries to benchmarks against
#include "benchmarks/thirdparty/nlohmann_json.hpp"    // nlohmann
#include "benchmarks/thirdparty/picojson.h"           // picojson
#include "benchmarks/thirdparty/rapidjson/document.h" // rapidjson
#include "benchmarks/thirdparty/rapidjson/prettywriter.h"
#include "benchmarks/thirdparty/rapidjson/stringbuffer.h"
#include "benchmarks/thirdparty/rapidjson/writer.h"

// Standard headers
// None

// ____________________ IMPLEMENTATION ____________________

// ======================
// --- Preparing data ---
// ======================

void generate_test_data() {
    // Generate string JSON data
    std::vector<std::string> strings(120'000);

    for (auto& str : strings) {

        // Create string of random letters
        str = std::string(random::uniform(0, 120), '0');
        for (auto& ch : str) ch = random::uniform('a', 'z');

        // Add some escapes at random
        if (!str.empty() && random::uniform_double() < 0.1) {
            const auto max_pos = static_cast<random::Uint>(str.size() - 1);
            str.at(random::uniform_uint(0, max_pos)) =
                random::choose({'"', '\\', '/', '\b', '\f', '\n', '\r', '\t'});
        }
    }

    json::Node(strings).to_file("benchmarks/data/strings.json");

    // Generate numeric JSON data
    std::vector<double> numbers(180'000);

    for (auto& num : numbers) num = random::choose({random::uniform(-10., 10.), random::uniform(-1e108, 1e108)});

    json::Node(numbers).to_file("benchmarks/data/numbers.json");
}

// =================
// --- Benchmark ---
// =================

void benchmark_parse_serialize_on_data(const std::string& filepath) {

    constexpr auto parsing_target_minimized      = "temp/parsing_target_minimized.json";
    constexpr auto parsing_target_prettified     = "temp/parsing_target_prettified.json";
    constexpr auto serializing_target_minimized  = "temp/serializing_target_minimized.json";
    constexpr auto serializing_target_prettified = "temp/serializing_target_prettified.json";

    const std::string filename = filepath.substr(filepath.find_last_of('/') + 1);
    std::cout << "\n\n====== BENCHMARKING ON DATA: `" << filename << "` ======\n";

    // Create in-memory JSONs that will be used to benchmark serializing
    const std::string string_buffer = (std::ostringstream() << std::ifstream(filepath).rdbuf()).str();

    // utl::json
    json::Node     json_utl = json::from_string(string_buffer);
    // nlohmann
    nlohmann::json json_nlohmann;
    std::ifstream(filepath) >> json_nlohmann;
    // PicoJSON
    picojson::value json_picojson;
    picojson::parse(json_picojson, string_buffer);
    // RapidJSON
    rapidjson::Document json_rapidjson;
    json_rapidjson.Parse(string_buffer.data());

    // Create minimized & prettified JSON file that will be used to benchmark parsing
    json_utl.to_file(parsing_target_minimized, json::Format::MINIMIZED);
    json_utl.to_file(parsing_target_prettified, json::Format::PRETTY);

    // Set global benchmark options
    bench.minEpochIterations(4).timeUnit(1ms, "ms");

    // Benchmark parsing (minimized)
    bench.title("Parsing minimized JSON").relative(true).warmup(10);

    benchmark("utl::json", [&]() {
        const auto json = json::from_file(parsing_target_minimized);
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    benchmark("nlohmann", [&]() {
        nlohmann::json json;
        std::ifstream(parsing_target_minimized) >> json;
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    benchmark("PicoJSON", [&]() {
        picojson::value   json;
        const std::string buffer = (std::ostringstream() << std::ifstream(parsing_target_minimized).rdbuf()).str();
        picojson::parse(json, buffer);
        // not sure how to read files directly with PicoJSON (without parsing into a string buffer first),
        // so we'll assume the user goes for the simplest way of getting file contents into a string
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    benchmark("RapidJSON", [&]() {
        rapidjson::Document json;
        const std::string   buffer = (std::ostringstream() << std::ifstream(parsing_target_minimized).rdbuf()).str();
        json.Parse(buffer.data());
        // not sure how to read files directly with RapidJSON (without parsing into a string buffer first),
        // so we'll assume the user goes for the simplest way of getting file contents into a string
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    // Benchmark parsing (prettified)
    bench.title("Parsing prettified JSON").relative(true);

    benchmark("utl::json", [&]() {
        const auto json = json::from_file(parsing_target_prettified);
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    benchmark("nlohmann", [&]() {
        nlohmann::json json;
        std::ifstream(parsing_target_prettified) >> json;
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    benchmark("PicoJSON", [&]() {
        picojson::value   json;
        const std::string buffer = (std::ostringstream() << std::ifstream(parsing_target_prettified).rdbuf()).str();
        picojson::parse(json, buffer);
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    benchmark("RapidJSON", [&]() {
        rapidjson::Document json;
        const std::string   buffer = (std::ostringstream() << std::ifstream(parsing_target_prettified).rdbuf()).str();
        json.Parse(buffer.data());
        DO_NOT_OPTIMIZE_AWAY(json);
    });

    // Benchmark serializing (minimized)
    bench.title("Serializing minimized JSON").relative(true);

    benchmark("utl::json", [&]() { json_utl.to_file(serializing_target_minimized, json::Format::MINIMIZED); });

    benchmark("nlohmann", [&]() { std::ofstream(serializing_target_minimized) << json_nlohmann.dump(); });

    benchmark("PicoJSON", [&]() { std::ofstream(serializing_target_minimized) << json_picojson.serialize(); });

    benchmark("RapidJSON", [&]() {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        json_rapidjson.Accept(writer);
        std::ofstream(serializing_target_minimized) << buffer.GetString();
        // what an API, through I guess that's understandable, considering the don't use any std headers
    });

    // Benchmark serializing (prettified)
    bench.title("Serializing prettified JSON").relative(true);

    benchmark("utl::json", [&]() { json_utl.to_file(serializing_target_prettified, json::Format::PRETTY); });

    benchmark("nlohmann", [&]() { std::ofstream(serializing_target_prettified) << json_nlohmann.dump(4); });

    benchmark("PicoJSON", [&]() { std::ofstream(serializing_target_prettified) << json_picojson.serialize(true); });

    benchmark("RapidJSON", [&]() {
        rapidjson::StringBuffer                          buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        json_rapidjson.Accept(writer);
        std::ofstream(serializing_target_prettified) << buffer.GetString();
    });
}

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    // generate_test_data();

    benchmark_parse_serialize_on_data("benchmarks/data/strings.json");
    benchmark_parse_serialize_on_data("benchmarks/data/numbers.json");
    benchmark_parse_serialize_on_data("benchmarks/data/database.json");
    benchmark_parse_serialize_on_data("benchmarks/data/twitter.json");
    benchmark_parse_serialize_on_data("benchmarks/data/random.json");
    benchmark_parse_serialize_on_data("benchmarks/data/canada.json");
    benchmark_parse_serialize_on_data("benchmarks/data/apache_builds.json");
}