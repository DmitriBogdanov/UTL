#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    // Verbose async file logger
    auto logger = log::logger{
        log::sink<
            log::policy::type::file,
            log::policy::level::trace,
            log::policy::color::none,
            log::policy::format::full,
            log::policy::buffering::fixed,
            log::policy::flushing::async,
            log::policy::threading::safe
        >{"latest.log"}
    };
    
    logger.info("Message 1");
    logger.note("Message 2");
    logger.warn("Message 3");
}