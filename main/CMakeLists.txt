set(MAIN_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(COMPONENT_ADD_INCLUDEDIRS
    ${MAIN_ROOT}/
    ${MAIN_ROOT}/inc
    ${MAIN_ROOT}/HAL
    ${MAIN_ROOT}/APP
    )
file(GLOB_RECURSE SRCS
     ${MAIN_ROOT}/*.cpp
     ${MAIN_ROOT}/*.c
     )

set(COMPONENT_SRCS ${SRCS})

message(STATUS "main use components = ${SRCS}")

register_component()

target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-missing-field-initializers"
)