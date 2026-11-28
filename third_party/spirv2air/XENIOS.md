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
- `spirv2air.h` `GraphicsBindings::sharedMemoryBuffer`, `directTextures` and `constantTextureIndices`,
  and `Options::preciseMath` (`translate.cpp`: `buildSignature`, `buildEntry`, `loadResource`,
  `fast`): Metal argument-table bindings for the shared memory StorageBuffer (`air.buffer`) and for
  textures and samplers (`air.texture`/`air.sampler`, sampler index = binding minus the module's
  lowest sampler binding), with no index records or heaps, as compute modules already have; the
  index records read through a constant pointer; and the precise math intrinsics, which compute
  modules call, in graphics modules. All off by default, so the default output is unchanged. iOS
  16's compiler fails on the pointers and handles these replace (below).
- `spirv2air.h` `Options::declarationAttributes` (`translate.cpp` `declare`): which of the attributes
  Apple's current compiler writes on intrinsic declarations are written: all (0, the default), all but
  `mustprogress` (1), only `nounwind`, `readnone`/`readonly`, `argmemonly` and `convergent` (2), or
  none and no `local_unnamed_addr` (3).
- `spirv2air.h` `kVersion`: "spirv2air 1.0.8 xenios-ios16.3".

Checked on iOS 16.5.1 (A16): Metal accepts the libraries and functions (build 9691). Pipelines:
XeniOS's self-test (metal_spirv2air_self_test in msl_shader.cc, shaders in
src/xenia/gpu/metal/spirv2air_self_test_spirv.h) on builds 9697 and 9698 built every module with no
call (vertex shaders with outputs or reading shared memory through the argument buffer's device
pointer, pixel shaders writing a constant, reading inputs or a constant buffer, with early fragment
tests) and failed every module calling an intrinsic ("Compiler encountered an internal error"): a
float-to-int conversion (in 14 ms), math functions (fast and precise), discard, a texture sample
(with heaps or direct bindings), unpack, and every game shader. Bound directly or not made no
difference. Only a module with a call has intrinsic declarations, and so an attribute table:
`mustprogress` (LLVM 12), `willreturn`, `nofree`, `nosync`, `nounwind`, `readnone`, ... as Apple's
current compiler writes them. declarationAttributes 1-3 test whether iOS 16's compiler is the one
that can't take them. Failures took multiples of about 10 s (one hung for an hour), so the
compiler service retries a hung compile; the self-test now gives each pipeline 40 s.

`include/spirv/unified1` holds the
Khronos headers the translator includes (spirv.hpp from third_party/SPIRV-Cross, GLSL.std.450.h from
third_party/glslang, both MIT).
