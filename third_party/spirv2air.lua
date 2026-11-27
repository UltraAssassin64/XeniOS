-- spirv2air (third_party/spirv2air, see XENIOS.md): SPIR-V to Apple AIR and a
-- metallib, without the Metal Shading Language step. Apple targets only (the
-- metallib writer uses CommonCrypto).
group("third_party")
project("spirv2air")
  uuid("5c1d9a7e-3f42-4b8e-9d61-2a7f0c4e8b13")
  kind("StaticLib")
  language("C++")
  cppdialect("C++17")
  includedirs({
    "spirv2air/src",
    "spirv2air/src/spirv2air",
    "spirv2air/src/airbc",
  })
  -- translate.cpp includes <spirv/unified1/...> with angle brackets, which
  -- Xcode only searches in header (not user header) search paths.
  externalincludedirs({
    "spirv2air/include",
  })
  files({
    "spirv2air/src/airbc/airbc.cpp",
    "spirv2air/src/airbc/airbc.h",
    "spirv2air/src/metallib/metallib.cpp",
    "spirv2air/src/metallib/metallib.h",
    "spirv2air/src/spirv2air/*.cpp",
    "spirv2air/src/spirv2air/*.h",
  })
