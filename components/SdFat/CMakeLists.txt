set (COMPONENTS_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(COMPONENT_ADD_INCLUDEDIRS
    ${COMPONENTS_ROOT}/src
    ${COMPONENTS_ROOT}/src/common
    ${COMPONENTS_ROOT}/src/ExFatLib
    ${COMPONENTS_ROOT}/src/FatLib
    ${COMPONENTS_ROOT}/src/FsLib
    ${COMPONENTS_ROOT}/src/iostream
    ${COMPONENTS_ROOT}/src/SdCard
    )
file(GLOB_RECURSE  SRCS
    ${COMPONENTS_ROOT}/src/*.cpp
     )

set(COMPONENT_SRCS ${SRCS})

set(COMPONENT_REQUIRES
    "arduino-esp32"
    "esp_lcd"
)

message(STATUS "SdFat use components = ${SRCS}")

# 必须先注册组件
register_component()

# 然后在注册之后添加编译选项
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-missing-field-initializers"
)