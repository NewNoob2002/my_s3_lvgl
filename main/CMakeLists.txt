set(MAIN_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(COMPONENT_ADD_INCLUDEDIRS
    ${MAIN_ROOT}/
    ${MAIN_ROOT}/config
    ${MAIN_ROOT}/inc
    ${MAIN_ROOT}/HAL
    )
file(GLOB SRCS
     ${MAIN_ROOT}/*.cpp
     ${MAIN_ROOT}/HAL/*.cpp
     ${MAIN_ROOT}/lvgl_port/*.cpp
     ${MAIN_ROOT}/src/*.cpp
     )

set(COMPONENT_SRCS ${SRCS})


### If you use arduino-esp32 components, please activate next comment line.
# list(APPEND COMPONENT_REQUIRES arduino-esp32)


message(STATUS "main use components = ${SRCS}")

register_component()