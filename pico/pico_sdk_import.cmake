# This is a copy of <PICO_SDK_PATH>/external/pico_sdk_import.cmake

if (NOT PICO_SDK_PATH)
    set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../pico-sdk)
endif()

if (NOT EXISTS ${PICO_SDK_PATH}/pico_sdk_init.cmake)
    message(FATAL_ERROR "PICO_SDK_PATH not set correctly")
endif()

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)