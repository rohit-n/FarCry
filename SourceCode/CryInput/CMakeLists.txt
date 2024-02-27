project(CryInput)

add_definitions(-D_USRDLL -DCRYINPUT_EXPORTS)

if (NOT MSVC)
add_definitions(-DUSE_SDL_INPUT)
endif()

set(SRC_FILES
    "CryInput.cpp"
    "Input.cpp"
    "Joystick.cpp"
    "StdAfx.cpp"
    "XActionMap.cpp"
    "XActionMapManager.cpp"
)

if (MSVC)
set (SRC_FILES ${SRC_FILES}
    "XKeyboard.cpp"
    "XMouse.cpp"
)
else()
set (SRC_FILES ${SRC_FILES}
    "SDLKeyboard.cpp"
    "SDLMouse.cpp"
)
endif()

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/SourceCode/CryCommon
)

target_link_libraries(${PROJECT_NAME} SDL2)
