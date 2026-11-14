group("third_party")
project("boost_context")
  uuid("6b0c6b2d-6f0f-4d0b-9a4e-3a1c2a7f5d11")
  kind("StaticLib")
  language("C")
  -- Boost.Context fcontext_t fiber primitives (BSL-1.0). Only the three asm
  -- routines are built; they are declared in src/xenia/base/threading_fiber.cc.
  filter({"system:macosx or ios", "architecture:ARM64"})
    files({
      "boost_context/context/src/asm/make_arm64_aapcs_macho_gas.S",
      "boost_context/context/src/asm/jump_arm64_aapcs_macho_gas.S",
      "boost_context/context/src/asm/ontop_arm64_aapcs_macho_gas.S",
    })
  filter({"system:macosx", "architecture:x86_64"})
    files({
      "boost_context/context/src/asm/make_x86_64_sysv_macho_gas.S",
      "boost_context/context/src/asm/jump_x86_64_sysv_macho_gas.S",
      "boost_context/context/src/asm/ontop_x86_64_sysv_macho_gas.S",
    })
  filter({"system:linux or android", "architecture:ARM64"})
    files({
      "boost_context/context/src/asm/make_arm64_aapcs_elf_gas.S",
      "boost_context/context/src/asm/jump_arm64_aapcs_elf_gas.S",
      "boost_context/context/src/asm/ontop_arm64_aapcs_elf_gas.S",
    })
  filter({"system:linux or android", "architecture:x86_64"})
    files({
      "boost_context/context/src/asm/make_x86_64_sysv_elf_gas.S",
      "boost_context/context/src/asm/jump_x86_64_sysv_elf_gas.S",
      "boost_context/context/src/asm/ontop_x86_64_sysv_elf_gas.S",
    })
  filter({})
