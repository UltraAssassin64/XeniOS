project_root = "../../../.."
include(project_root.."/tools/build")

group("src")
project("xenia-hid-portal")
  uuid("897b6f26-b0c1-43c1-a013-a37e7b9634fd")
  kind("StaticLib")
  language("C++")
  links({
    "xenia-base",
    "xenia-hid",
    "libusb",
  })
  local_platform_files()
