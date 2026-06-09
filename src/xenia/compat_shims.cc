#include <cstdlib>

extern "C" {
    __attribute__((weak, visibility("default")))
    [[noreturn]] void quick_exit(int status) noexcept {
        _Exit(status);
    }
}