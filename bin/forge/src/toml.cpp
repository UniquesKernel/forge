#include "toml.hpp"
#include "assert.hpp"
#include "status.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
void safe_fclose(FILE** ptr) {
        if (ptr == NULL || *ptr == NULL) {
                return;
        }
        fclose(*ptr);
}

void safe_close(int* fd_ptr) {
        if (fd_ptr && *fd_ptr >= 0) {
                close(*fd_ptr);
        }
}
} // namespace

namespace forge::toml {

[[nodiscard]]
Error map_config(TomlSource* const toml_source) noexcept {
        REQUIRE(toml_source != nullptr, Error::NULL_PARAMETER);
        REQUIRE(toml_source->source == nullptr, Error::INVALID_ARGUMENT);
        REQUIRE(toml_source->size == 0, Error::INVALID_ARGUMENT);

        int fd DEFER(safe_close) = open("forge.toml", O_RDONLY);

        REQUIRE(fd != -1, Error::CONFIG_FILE_ACCESS_ERROR);

        struct stat st;
        int         fs_result = fstat(fd, &st);

        REQUIRE(fs_result >= 0, Error::CONFIG_FILE_ACCESS_ERROR);
        REQUIRE(st.st_size > 0, Error::EMPTY_CONFIG_FILE_ERROR);

        const char* source = (const char*)mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

        REQUIRE(source != MAP_FAILED, Error::CONFIG_FILE_ACCESS_ERROR);

        (void)madvise((void*)source, st.st_size, MADV_SEQUENTIAL);

        toml_source->source = source;
        toml_source->size   = st.st_size;

        return Error::OK;
}

} // namespace forge::toml
