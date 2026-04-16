#ifndef FORGE_TOML_HPP
#define FORGE_TOML_HPP

#include "status.hpp"
#include <sys/types.h>

namespace forge::toml {
/*!
  @brief Toml Source representation

  @invariant size > 0

  @pre The struct must be initialized via `map_config` before any field is read
  @post After successful initialization: `source` points to a valid read-only memory region of exactly `size` bytes

  @par Memory Layout
  | Name   | Type        | Size    |
  |--------|-------------|---------|
  | source | const char* | 8 bytes |
  | size   | off_t       | 8 bytes |

  @par Total Size
  16 bytes
 */
struct TomlSource {
        const char* source = nullptr;
        off_t       size   = 0;
};
static_assert(sizeof(TomlSource) == 16, "Toml Source struct is expected to occupy 16 bytes");
static_assert(alignof(TomlSource) == alignof(void*),
              "Toml Source struct is expected to be aligned to the pointer alignment");

/*!
  @brief Initialize a `TomlSource` struct

  @pre `toml_source` must not be null
  @post On `OK`: `toml_source->source` points to a valid read-only memory region and `toml_source->size > 0`
  @post On error: the contents of `*toml_source` are unspecified and must not be read

  @param[out] toml_source		the toml source to initialize
  @return Error enumeration where `OK` indicate success and all other values indicate an error

  @note The function does not throw - but can abort if internal invariants are violated
 */
[[nodiscard]]
Error map_config(TomlSource* toml_source) noexcept;
} // namespace forge::toml
#endif
