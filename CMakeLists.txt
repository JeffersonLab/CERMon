# CMakeLists.txt
#
# Build files for the RasterMon program.
#
# Author: Maurik Holtrop
#
cmake_minimum_required(VERSION 3.20)
project(CERMon VERSION 0.1 LANGUAGES CXX)
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY True)

set(CMAKE_CXX_STANDARD 17)

# include(cmake/FindElog.cmake)   ## ELOG is *dead* for C++17 code. Blame OPS for being 11 years behind in C++.

if(${ELOG_FOUND})
    message(STATUS "The elog library was found at ${ELOG_LIBRARIES} + ${ELOG_INCLUDE_DIR}")
    set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations -fpermissive")
endif()

# find_package(EvioTool CONFIG)  # EvioTool will also find fEvio.

if(${EvioTool_FOUND})
    message(STATUS "Found an existing installation of EvioTool. Will use that.")
    get_target_property(EvioTool_DIR EvioTool INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "EvioTool found at ${EvioTool_DIR}")
    get_target_property(evio_DIR evio INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "evio found at ${evio_DIR}")
    get_target_property(et_DIR et INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "et found at ${et_DIR}")
    #
    # The RootMacros.cmake is broken. It adds the INCLUDE_DIRECTORIES for a target, not the INTERFACE_INCLUDE_DIRECTORIES
    #
    set_target_properties(EvioTool PROPERTIES INCLUDE_DIRECTORIES ${EvioTool_DIR})
    set_target_properties(evio PROPERTIES INCLUDE_DIRECTORIES ${evio_DIR})
    set_target_properties(et PROPERTIES INCLUDE_DIRECTORIES ${et_DIR})

else()
    message(STATUS "EvioTool not found. Will build locally.")
    add_subdirectory(EvioTool)
endif()

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
# This *must* come after EvioTool, because EvioTool imports ROOT without the Gui component.

find_package(ROOT REQUIRED COMPONENTS Core Gui)
message(STATUS "ROOT found at: ${ROOT_DIR}")

set(default_build_type "RELEASE")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# For dictionary compile step to find include files.
# The target properties for include directories need to be explicitly added to the include_directories.

if(${ELOG_FOUND})
    include_directories(${ELOG_INCLUDE_DIR})
endif()

add_subdirectory(CERMon)

# find_package(ccdb)
# add_executable(test_tt test_tt ccdb)

