// compat_shims.h
#include <cstdlib>

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void quick_exit(int status) noexcept __attribute__((weak));

#ifdef __cplusplus
}
#endif