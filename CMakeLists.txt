cmake_minimum_required(VERSION 3.20)
set(VERSION "0.0.1")
project(CNCpp VERSION ${VERSION} LANGUAGES CXX)

#   ____       _
#  / ___|  ___| |_  __   ____ _ _ __ ___
#  \___ \ / _ \ __| \ \ / / _` | '__/ __|
#   ___) |  __/ |_   \ V / (_| | |  \__ \
#  |____/ \___|\__|   \_/ \__,_|_|  |___/

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
  set(CMAKE_POLICY_VERSION_MINIMUM "3.5")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(BUILD_FLAG "RELEASE_BUILD")
else()
  set(BUILD_FLAG "DEBUG_BUILD")
  add_compile_definitions(DEBUG=1)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(MAIN_DIR ${SRC_DIR}/main)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/usr CACHE PATH "Install path prefix" FORCE)

include(CTest)

option(BUILD_QUICK_TESTS "Build quick developer test executables" OFF)

message(STATUS "SRC_DIR: ${SRC_DIR}")
message(STATUS "MAIN_DIR: ${MAIN_DIR}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX ".*/_.*\\.cpp$")


#   _____         _   ____            _
#  |___ / _ __ __| | |  _ \ __ _ _ __| |_ _   _
#    |_ \| '__/ _` | | |_) / _` | '__| __| | | |
#   ___) | | | (_| | |  __/ (_| | |  | |_| |_| |
#  |____/|_|  \__,_| |_|   \__,_|_|   \__|\__, |
#                                         |___/

include(FetchContent)

message(STATUS "Fetching dependencies...")

FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        11.0.2
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(rang
  GIT_REPOSITORY https://github.com/agauniyal/rang.git
  GIT_TAG        v3.2
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(toml
  GIT_REPOSITORY https://github.com/pbosetti/tomlplusplus.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)

FetchContent_Declare(keystroker
  GIT_REPOSITORY https://github.com/pbosetti/keystroker.git
  GIT_SHALLOW    TRUE
  GIT_TAG HEAD
)

FetchContent_MakeAvailable(fmt rang json toml keystroker)

message(STATUS "Dependencies fetched.")

include_directories(
  ${fmt_SOURCE_DIR}/include
  ${rang_SOURCE_DIR}/include
  ${json_SOURCE_DIR}/include
  ${toml_SOURCE_DIR}/include
  ${keystroker_SOURCE_DIR}/include
  # /opt/homebrew/include
)

add_compile_options(-Wno-deprecated-literal-operator)

find_package(Mads REQUIRED)
message(STATUS "Found Mads: ${Mads_DIR}")

#    ____             __ _                          _    __ _ _
#   / ___|___  _ __  / _(_) __ _ _   _ _ __ ___  __| |  / _(_) | ___  ___
#  | |   / _ \| '_ \| |_| |/ _` | | | | '__/ _ \/ _` | | |_| | |/ _ \/ __|
#  | |__| (_) | | | |  _| | (_| | |_| | | |  __/ (_| | |  _| | |  __/\__ \
#   \____\___/|_| |_|_| |_|\__, |\__,_|_|  \___|\__,_| |_| |_|_|\___||___/
#                          |___/

# configure_file(
#   ${SRC_DIR}/defines.hpp.in
#   ${SRC_DIR}/defines.hpp
# )

#  ____                                        _        _   _             
# |  _ \  ___   ___ _   _ _ __ ___   ___ _ __ | |_ __ _| |_(_) ___  _ __  
# | | | |/ _ \ / __| | | | '_ ` _ \ / _ \ '_ \| __/ _` | __| |/ _ \| '_ \ 
# | |_| | (_) | (__| |_| | | | | | |  __/ | | | || (_| | |_| | (_) | | | |
# |____/ \___/ \___|\__,_|_| |_| |_|\___|_| |_|\__\__,_|\__|_|\___/|_| |_|
#                     

# Now the Doxygen documentation and Doxyfile are placed in the build directory, 
# so they won't clutter the source one. However, it is ignored by git, therefore
# it won't be added to the repository: so at the and this configuration MUST be 
#changed.
  
find_package(Doxygen)

if(DOXYGEN_FOUND)
  set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
  set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
  set(DOXYGEN_INPUT "${SRC_DIR}")
  set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc")
  set(DOXYGEN_OUTPUT_DIR "${DOXYGEN_OUTPUT_DIRECTORY}")
  set(DOXYGEN_FILE_PATTERNS "*.hpp")
  set(DOXYGEN_RECURSIVE YES)

  configure_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeDoxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
  )

  add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYGEN_OUTPUT_DIR}
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
else()
  message(STATUS "Doxygen not found: 'doc' target will not be available")
endif()

#   _____                    _
#  |_   _|_ _ _ __ __ _  ___| |_ ___
#    | |/ _` | '__/ _` |/ _ \ __/ __|
#    | | (_| | | | (_| |  __/ |_\__ \
#    |_|\__,_|_|  \__, |\___|\__|___/
#                 |___/

# main Library
file(GLOB HEADERS ${SRC_DIR}/*.hpp)

add_library(cncpp STATIC ${SRC_FILES})
target_include_directories(cncpp PUBLIC ${SRC_DIR})
target_link_libraries(cncpp PUBLIC fmt::fmt)
target_compile_definitions(cncpp PRIVATE CNCPP_TEST_PROGRAM_MAIN)

if(BUILD_TESTING)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)
endif()

if(BUILD_QUICK_TESTS)
  set(QUICK_TEST_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/quick_tests")

  # Point test
  add_executable(point_test ${SRC_DIR}/point.cpp)
  target_compile_definitions(point_test PRIVATE CNCPP_POINT_TEST_MAIN)
  target_link_libraries(point_test PRIVATE cncpp fmt::fmt)
  set_target_properties(point_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${QUICK_TEST_OUTPUT_DIR}")

  # Block test
  add_executable(block_test ${SRC_DIR}/block.cpp)
  target_compile_definitions(block_test PRIVATE CNCPP_BLOCK_TEST_MAIN)
  target_link_libraries(block_test PRIVATE cncpp fmt::fmt)
  set_target_properties(block_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${QUICK_TEST_OUTPUT_DIR}")

  # Program test
  add_executable(program_test ${SRC_DIR}/program.cpp)
  target_compile_definitions(program_test PRIVATE CNCPP_PROGRAM_TEST_MAIN)
  target_link_libraries(program_test PRIVATE cncpp fmt::fmt)
  set_target_properties(program_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${QUICK_TEST_OUTPUT_DIR}")

  # Machine test
  add_executable(machine_test ${SRC_DIR}/machine.cpp)
  target_compile_definitions(machine_test PRIVATE CNCPP_MACHINE_TEST_MAIN)
  target_link_libraries(machine_test PRIVATE cncpp fmt::fmt)
  set_target_properties(machine_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${QUICK_TEST_OUTPUT_DIR}")
endif()

add_executable(cnc ${SRC_DIR}/main/cnc.cpp)
target_link_libraries(cnc PRIVATE cncpp Mads::Mads keystroker rang fmt)

#   ___           _        _ _
#  |_ _|_ __  ___| |_ __ _| | |
#   | || '_ \/ __| __/ _` | | |
#   | || | | \__ \ || (_| | | |
#  |___|_| |_|___/\__\__,_|_|_|
#

install(TARGETS ${TARGET_LIST}
  BUNDLE DESTINATION bin
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)
