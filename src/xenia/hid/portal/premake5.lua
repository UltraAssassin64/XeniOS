project_root = "../../../.."
include(project_root.."/tools/build")

group("src")
project("xenia-hid-portal")
  uuid("897a6f26-a0c1-53c1-b013-b37e7b9634fd")
  kind("StaticLib")
  language("C++")
  links({
    "xenia-base",
    "xenia-hid",
    "libusb",
  })
  local_platform_files()
