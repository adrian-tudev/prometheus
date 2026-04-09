cmake_minimum_required(VERSION 3.20)

project(prometheus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER ${CONFIG} CONFIG_UPPER)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_SOURCE_DIR}/bin)
endforeach()

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set(GUI_DIR ${CMAKE_SOURCE_DIR}/gui)

set(CORE_NO_MAIN_SOURCES
  ${SRC_DIR}/bitboard.cpp
  ${SRC_DIR}/engine.cpp
  ${SRC_DIR}/movegen.cpp
  ${SRC_DIR}/position.cpp
  ${SRC_DIR}/ui.cpp
  ${SRC_DIR}/utils.cpp
)

add_executable(prometheus
  ${SRC_DIR}/main.cpp
  ${CORE_NO_MAIN_SOURCES}
)
target_include_directories(prometheus PRIVATE ${SRC_DIR})

add_executable(perft
  ${TEST_DIR}/perft.cpp
  ${CORE_NO_MAIN_SOURCES}
)
target_include_directories(perft PRIVATE ${SRC_DIR})

# Keep perft fast regardless of the global build type.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(perft PRIVATE -O3 -DNDEBUG)
elseif(MSVC)
  target_compile_options(perft PRIVATE /O2 /DNDEBUG)
endif()

option(PROMETHEUS_BUILD_GUI "Build SDL GUI target" OFF)

if(PROMETHEUS_BUILD_GUI)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
  pkg_check_modules(SDL3_IMAGE REQUIRED IMPORTED_TARGET sdl3-image)

  set(GUI_CORE_SOURCES
    ${SRC_DIR}/bitboard.cpp
    ${SRC_DIR}/movegen.cpp
    ${SRC_DIR}/position.cpp
    ${SRC_DIR}/utils.cpp
  )

  add_executable(app
    ${GUI_DIR}/main.cpp
    ${GUI_DIR}/gui.cpp
    ${GUI_CORE_SOURCES}
  )
  target_include_directories(app PRIVATE ${SRC_DIR})
  target_link_libraries(app PRIVATE PkgConfig::SDL3 PkgConfig::SDL3_IMAGE)

  add_custom_target(gui DEPENDS app)

  add_custom_target(run-gui
    COMMAND $<TARGET_FILE:app>
    DEPENDS app
    USES_TERMINAL
  )
endif()

add_custom_target(run
  COMMAND $<TARGET_FILE:prometheus>
  DEPENDS prometheus
  USES_TERMINAL
)

add_custom_target(run-perft
  COMMAND $<TARGET_FILE:perft>
  DEPENDS perft
  USES_TERMINAL
)
