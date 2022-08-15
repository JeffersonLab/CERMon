project(CERMonLib VERSION 0.1 LANGUAGES CXX)

#
# Some items need control.
#
if (NOT DEFINED DEFAULT_HISTOGRAM_PATH)
    set(DEFAULT_HISTOGRAM_PATH "/home/clasrun/cermon")
endif()
message(STATUS "DEFAULT_HISTOGRAM_PATH set to ${DEFAULT_HISTOGRAM_PATH}")
if (NOT DEFINED CLI_LOGENTRY_PROGRAM)
    set(CLI_LOGENTRY_PROGRAM "/home/clasrun/CERMon/logentry")
endif()
message(STATUS "CLI_LOGENTRY_PROGRAM set to ${CLI_LOGENTRY_PROGRAM}")


message(STATUS "Current PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}")
include_directories(${PROJECT_SOURCE_DIR}/CERMon)

set(SOURCE_FILES
        CERMon.cxx
        CERMonEventInfo.cxx
        CERMonGui.cxx
        CERMonDefaultConfig.cxx
        CERMonEvioTool.cxx
        CERMonHists.cxx
        ConfigInfo.cxx
        ConfigPanel.cxx
        LogbookEntry.cxx
        )

add_library(CERMonLib SHARED ${SOURCE_FILES})

# DEPENDENCIES EvioTool evio et
target_link_libraries(CERMonLib ${ROOT_LIBRARIES} EvioTool evio et)
ROOT_GENERATE_DICTIONARY(CERMonLib_dict CERMonLib.h LINKDEF CERMonLibLinkDef.h MODULE CERMonLib)

set_property(SOURCE logbookentry.cxx PROPERTY COMPILE_DEFINITIONS  DEFAULT_HISTOGRAM_PATH="${DEFAULT_HISTOGRAM_PATH}")

# target_compile_definitions(CERMonLib PRIVATE DEFAULT_HISTOGRAM_PATH=\"${DEFAULT_HISTOGRAM_PATH}\")
# target_compile_definitions(CERMonLib PRIVATE CLI_LOGENTRY_PROGRAM=\"${CLI_LOGENTRY_PROGRAM}\")

list(APPEND ADDITIONAL_CLEAN_FILES CERMonLib_dict.cxx)

add_executable(CERMon CERMon.cxx)
target_link_libraries(CERMon CERMonLib )
install(TARGETS CERMonLib  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libCERMonLib_dict_rdict.pcm  DESTINATION lib)
install(TARGETS CERMon DESTINATION bin)
