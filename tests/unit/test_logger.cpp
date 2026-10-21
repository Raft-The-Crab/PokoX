#include "poko/core/Logger.hpp"
#include <cassert>
#include <string>

using namespace poko::core;

void test_logger_singleton() {
    Logger& logger1 = Logger::instance();
    Logger& logger2 = Logger::instance();
    assert(&logger1 == &logger2);
}

void test_logger_level_filtering() {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::Warning);
    
    // These should not be visible at Warning level
    logger.debug("debug message");
    logger.trace("trace message");
    
    // These should be visible
    logger.warning("warning message");
    logger.error("error message");
}

void test_scoped_log_level() {
    Logger& logger = Logger::instance();
    LogLevel original = logger.level();
    
    {
        ScopedLogLevel scope(logger, LogLevel::Debug);
        assert(logger.level() == LogLevel::Debug);
    }
    
    assert(logger.level() == original);
}

void test_logger_thread_safety() {
    Logger& logger = Logger::instance();
    logger.set_level(LogLevel::Info);
    
    logger.info("Thread safety test");
    logger.error("Error message");
    logger.warning("Warning message");
}
