#ifndef FORGE_ERROR_STATUS_HPP
#define FORGE_ERROR_STATUS_HPP

#include "types.hpp"

/**
 * @brief A list of error codes that can be used to indicate program errors
 */
enum Error : forge::u8 {
        OK                 = 0,
        INVALID_DIMENSIONS = 1,
        INVALID_ARGUMENTS  = 2,
        NULL_PARAMETER     = 3,
        OUT_OF_MEMORY      = 4,
};

#endif // !ANVIL_ERROR_STATUS_HPP
