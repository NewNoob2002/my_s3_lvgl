set (COMPONENTS_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(COMPONENT_ADD_INCLUDEDIRS
    ${COMPONENTS_ROOT}/src
    ${COMPONENTS_ROOT}/src/font
    ${COMPONENTS_ROOT}/src/canvas
    ${COMPONENTS_ROOT}/src/databus
    ${COMPONENTS_ROOT}/src/display
    )
file(GLOB  SRCS
    ${COMPONENTS_ROOT}/src/*.cpp
    ${COMPONENTS_ROOT}/src/canvas/*.cpp
    ${COMPONENTS_ROOT}/src/databus/*.cpp
    ${COMPONENTS_ROOT}/src/display/*.cpp
     )

set(COMPONENT_SRCS ${SRCS})

set(COMPONENT_REQUIRES
    "arduino-esp32"
    "esp_lcd"
)

message(STATUS "Arduino_GFX use components = ${SRCS}")

# 必须先注册组件
register_component()

# 然后在注册之后添加编译选项
target_compile_options(${COMPONENT_LIB} PRIVATE 
    "-Wno-missing-field-initializers"
)