find_package(Qt5 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)

set(src main.cpp octopihelper.cpp ../src/argumentlist.cpp)

set(header octopihelper.h ../src/argumentlist.h)

add_executable(octphelper ${src} ${header})
target_compile_definitions(octphelper PRIVATE QT_DEPRECATED_WARNINGS QT_USE_QSTRINGBUILDER QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_URL_CAST_FROM_STRING QT_NO_CAST_FROM_BYTEARRAY QT_NO_FOREACH)
target_include_directories(octphelper PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
target_link_libraries(octphelper PRIVATE Qt5::Core Qt5::Network)
install(TARGETS octphelper RUNTIME DESTINATION lib/octopi LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)
