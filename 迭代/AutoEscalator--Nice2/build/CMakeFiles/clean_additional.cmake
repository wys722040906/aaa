# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "app/CMakeFiles/elevator_system_autogen.dir/AutogenUsed.txt"
  "app/CMakeFiles/elevator_system_autogen.dir/ParseCache.txt"
  "app/elevator_system_autogen"
  )
endif()
