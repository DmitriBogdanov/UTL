#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    // Verbose async file logger
    auto logger = log::Logger{
        log::Sink<
            log::policy::Type::FILE,
            log::policy::Level::TRACE,
            log::policy::Color::NONE,
            log::policy::Format::FULL,
            log::policy::Buffering::TIMED,
            log::policy::Flushing::SYNC,
            log::policy::Threading::UNSAFE
        >{"latest.log"}
    };
    
    logger.info("Message 1");
    logger.note("Message 2");
    logger.warn("Message 3");
}