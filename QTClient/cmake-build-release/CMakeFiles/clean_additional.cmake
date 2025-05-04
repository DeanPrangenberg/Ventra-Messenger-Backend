# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/CryptGuard_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/CryptGuard_autogen.dir/ParseCache.txt"
  "CryptGuard_autogen"
  )
endif()
