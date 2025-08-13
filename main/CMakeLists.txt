set (MAIN_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(COMPONENT_ADD_INCLUDEDIRS
    ${MAIN_ROOT}/inc
    )
file(GLOB SRCS
     ${MAIN_ROOT}/src/*.cpp
     ${MAIN_ROOT}/*.cpp
     )

set(COMPONENT_SRCS ${SRCS})


### If you use arduino-esp32 components, please activate next comment line.
# list(APPEND COMPONENT_REQUIRES arduino-esp32)


message(STATUS "main use components = ${SRCS}")

register_component()