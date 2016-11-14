
#pragma once

#include <string>

/*
 * Defines some thread-safe logging functions.
 *
 * NOTE: Only thread safe is these are the only functions operating on stdout
 *       and stderr.
 */
namespace logger {

    /**
     * Log a regular message to stdout.
     */
    void
    log(std::string msg);

    /**
     * Log an error message to stderr.
     */
    void
    error(std::string msg);
}

