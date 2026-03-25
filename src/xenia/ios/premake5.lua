project_root = "../../.."
include(project_root.."/tools/build")

group("src")
project("xenia-ios")
  uuid("99a4ef39-b441-541f-9e33-9efe6a5ef702")
  kind("StaticLib")
  language("C++")
  links({
    "xenia-base",
    "xenia-cpu",
  })
  local_platform_files()
