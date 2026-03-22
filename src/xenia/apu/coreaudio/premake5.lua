project_root = "../../../.."
include(project_root.."/tools/build")

group("src")
project("xenia-apu-coreaudio")
  uuid("173b4e8b-841b-51d7-8455-5b41abc73c13")
  kind("StaticLib")
  language("C++")
  links({
      "xenia-apu",
      "xenia-base",
      "AudioToolbox.framework",
      "AVFoundation.framework",
      "AudioUnit.framework"
  }) 
  local_platform_files()
  
  