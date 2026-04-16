#ifndef FORGE_ERROR_STATUS_HPP
#define FORGE_ERROR_STATUS_HPP

#include "types.hpp"

/**
 * @brief A list of error codes that can be used to indicate program errors
 */
enum class Error : forge::u8 {
        OK = 0,
        NULL_PARAMETER,
        INVALID_ARGUMENT,
        UNEXPECTED_EOF,
        CONFIG_FILE_ACCESS_ERROR,
        EMPTY_CONFIG_FILE_ERROR
};

/**
 * @brief Returns a human-readable string representation of an Error code.
 */
constexpr const char* error_to_string(Error err) {
        switch (err) {
        case Error::OK:
                return "OK";
        case Error::NULL_PARAMETER:
                return "NULL PARAMETER";
        case Error::INVALID_ARGUMENT:
                return "INVALID ARGUMENT";
        case Error::UNEXPECTED_EOF:
                return "UNEXPECTED END OF FILE";
        case Error::CONFIG_FILE_ACCESS_ERROR:
                return "CONFIG FILE ACCESS ERROR";
        case Error::EMPTY_CONFIG_FILE_ERROR:
                return "EMPTY CONFIG FILE ERROR";
        default:
                return "UNKNOWN ERROR";
        }
}

#endif // !ANVIL_ERROR_STATUS_HPP
