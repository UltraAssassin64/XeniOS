# spirv2air in XeniOS (iOS 16 backport)

Copied from https://github.com/xenios-jp/spirv2air at 84638cec9a935c09083a2d12fb3695de2a91ef65
(the commit XeniOS pins), BSD-3-Clause (LICENSE). Only `src/airbc`, `src/metallib`, `src/spirv2air`,
the README and `docs` are copied; the tests, CLI and CMake files are not used (built by
`third_party/spirv2air.lua`).

Changes in this copy, so its output targets iOS 16:

- `metallib.cpp` `make_target`: the iOS floor is 16 (upstream 18).
- `metallib.cpp` `air_version`: iOS 16 is AIR 2.5 and iOS 17 AIR 2.6 (upstream: 2.7 below 26), so
  the triple is `air64_v25-apple-ios16.0.0` and the metallib file version 2.6.
- `translate.cpp`: `air.language_version` is Metal 3.0 below AIR 2.6 (upstream always 3.1), and so is
  the function's `VERS` language version in the metallib (upstream the `metallib.h` default, 3.1;
  iOS 16 rejected every pipeline with "Function main is using language version 3.1 which is
  incompatible with this OS").
- `spirv2air.h` `kVersion`: "spirv2air 1.0.8 xenios-ios16.1".

Checked on iOS 16.5.1 (A16): Metal accepts the libraries and functions (build 9691). Not checked
yet: that it accepts every pipeline and intrinsic spirv2air emits (the AIR is otherwise identical to
upstream's iOS 18 output). `include/spirv/unified1` holds the
Khronos headers the translator includes (spirv.hpp from third_party/SPIRV-Cross, GLSL.std.450.h from
third_party/glslang, both MIT).
