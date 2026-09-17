group("third_party")

project("xbyak_aarch64")
  uuid("595f3f3e-f5e8-489a-bd0f-289d0495bc08")
  kind("StaticLib")
  language("C++")
  cppdialect("C++20")

  includedirs({
    "xbyak_aarch64",
    "src",
  })

  files({
    "src/xbyak_aarch64_impl.cpp",
    "src/xbyak_aarch64_impl.h",
    "src/util_impl.cpp",
    "src/util_impl.h",
    "src/util_impl_linux.h",
    "src/util_impl_mac.h",
    "src/util_impl_windows.h",
    "src/xbyak_aarch64_mnemonic.h",
    "src/err_impl.h",
    "xbyak_aarch64/xbyak_aarch64.h",
    "xbyak_aarch64/xbyak_aarch64_adr.h",
    "xbyak_aarch64/xbyak_aarch64_code_array.h",
    "xbyak_aarch64/xbyak_aarch64_err.h",
    "xbyak_aarch64/xbyak_aarch64_gen.h",
    "xbyak_aarch64/xbyak_aarch64_inner.h",
    "xbyak_aarch64/xbyak_aarch64_label.h",
    "xbyak_aarch64/xbyak_aarch64_meta_mnemonic.h",
    "xbyak_aarch64/xbyak_aarch64_mnemonic_def.h",
    "xbyak_aarch64/xbyak_aarch64_perf.h",
    "xbyak_aarch64/xbyak_aarch64_reg.h",
    "xbyak_aarch64/xbyak_aarch64_reg_manager.h",
    "xbyak_aarch64/xbyak_aarch64_util.h",
    "xbyak_aarch64/xbyak_aarch64_version.h",
  })