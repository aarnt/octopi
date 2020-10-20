find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)
find_package(Threads REQUIRED)
find_library(UTIL_LIBRARY NAMES util)

set(CMAKE_AUTOMOC ON)

file(GLOB TS_FILES LIST_DIRECTORIES false "${CMAKE_CURRENT_LIST_DIR}/translations/*.ts")
qt5_add_translation(qmFiles ${TS_FILES})

set(src main.cpp passworddialog.cpp sudo.cpp)

set(header passworddialog.h sudo.h)

set(ui passworddialog.ui)

qt5_wrap_ui(src ${ui})

add_executable(octopi-sudo ${src} ${header} ${qmFiles})
target_compile_definitions(octopi-sudo PRIVATE QT_DEPRECATED_WARNINGS QT_USE_QSTRINGBUILDER QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_URL_CAST_FROM_STRING QT_NO_CAST_FROM_BYTEARRAY)
target_include_directories(octopi-sudo PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
target_link_libraries(octopi-sudo PRIVATE Threads::Threads Qt5::Core Qt5::Gui Qt5::Widgets ${UTIL_LIBRARY})
install(TARGETS octopi-sudo RUNTIME DESTINATION lib/octopi LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)
