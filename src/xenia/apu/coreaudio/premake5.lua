project_root = "../../../.."
include(project_root.."/tools/build")

group("src")
project('xenia-apu-coreaudio')
    uuid("153b4e8b-813a-40e6-9366-4b51abc73c45")
    kind("StaticLib")
    language("C++")
    links({
        "xenia-apu",
        "xenia-base",
        "AudioToolbox.framework",
        "CoreAudio.framework",
    })
    
  filter ("platforms:macos or platforms:ios")
    buildoptions {
  "-fobjc-arc"
    }
    files {
    "src/xenia/apu/coreaudio/coreaudio_audio_driver.cc",
    "src/xenia/apu/coreaudio/coreaudio_audio_driver.h",
    "src/xenia/apu/coreaudio/coreaudio_audio_system.cc",
    "src/xenia/apu/coreaudio/coreaudio_audio_system.h",
  }
  local_platform_files()
  
  