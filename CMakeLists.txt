cmake_minimum_required(VERSION 3.5)
project(securitywrap C)

if (NOT DEFINED WRAP_EXECUTABLE)
  message(FATAL_ERROR "WRAP_EXECUTABLE must be specified")
endif()

if (NOT DEFINED RESET_UID)
  set(RESET_UID OFF)
endif()

if (NOT DEFINED RESET_GID)
  set(RESET_GID OFF)
endif()

if (NOT DEFINED SET_UID AND NOT RESET_UID AND DEFINED SET_REAL_UID)
  set(SET_UID ${SET_REAL_UID})
endif()

if (NOT DEFINED SET_GID AND NOT RESET_GID AND DEFINED SET_REAL_GID)
  set(SET_GID ${SET_REAL_GID})
endif()

if (DEFINED SET_UID AND RESET_UID)
  message(FATAL_ERROR "SET_UID and RESET_UID cannot be used together")
endif()

if (DEFINED SET_GID AND RESET_GID)
  message(FATAL_ERROR "SET_GID and RESET_GID cannot be used together")
endif()

message(STATUS "WRAP_EXECUTABLE: ${WRAP_EXECUTABLE}")
add_definitions(-DWRAP_EXECUTABLE="${WRAP_EXECUTABLE}")

if (DEFINED SET_UID)
  message(STATUS "SET_UID: ${SET_UID}")
  add_definitions(-DSET_UID="${SET_UID}")
elseif (RESET_UID)
  message(STATUS "RESET_UID: ON")
  add_definitions(-DRESET_UID=1)
endif()

if (DEFINED SET_REAL_UID)
  message(STATUS "SET_REAL_UID: ${SET_REAL_UID}")
  add_definitions(-DSET_REAL_UID="${SET_REAL_UID}")
endif()

if (DEFINED SET_GID)
  message(STATUS "SET_GID: ${SET_GID}")
  add_definitions(-DSET_GID="${SET_GID}")
elseif (RESET_GID)
  message(STATUS "RESET_GID: ON")
  add_definitions(-DRESET_GID=1)
endif()

if (DEFINED SET_REAL_GID)
  message(STATUS "SET_REAL_GID: ${SET_REAL_GID}")
  add_definitions(-DSET_REAL_GID="${SET_REAL_GID}")
endif()

cmake_path(GET WRAP_EXECUTABLE STEM BASENAME)

add_executable(${BASENAME} securitywrap.c)
install(TARGETS ${BASENAME} DESTINATION bin)
