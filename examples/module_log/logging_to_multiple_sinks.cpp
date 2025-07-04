#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    // Log everything to file
    log::add_file_sink("verbose.log").set_verbosity(log::Verbosity::TRACE);
    
    // Log meaningful events to a separate file with colors enabled for easier viewing
    log::add_file_sink("info.log").set_verbosity(log::Verbosity::INFO).set_colors(log::Colors::ENABLE);
    
    // Instead of calling 'set_...()' we can also pass arguments directly into 'add_..._sink()' function,
    // let's also append all logs to a persistent file that doesn't get rewritten between executions
    log::add_file_sink("history.log", log::OpenMode::APPEND).skip_header();
    
    // Add another file for logged messages only (no date/uptime/thread/callsite columns)
    log::Columns cols;
    cols.datetime = false;
    cols.uptime   = false;
    cols.thread   = false;
    cols.callsite = false;
    log::add_file_sink("messages.log").set_columns(cols);
    
    // Log warnings and errors to 'std::cerr'
    log::add_ostream_sink(std::cerr, log::Verbosity::WARN, log::Colors::ENABLE);
    
    // Log some stuff
    UTL_LOG_DTRACE("Some meaningless stuff"); // 'D' prefix means this will only compile in debug
    UTL_LOG_INFO("Some meaningful stuff");
    UTL_LOG_WARN("Some warning");
    UTL_LOG_ERR("Some error");
}