find_package(Qt5 REQUIRED COMPONENTS Core Xml Gui Widgets Network)
find_package(qtermwidget5 REQUIRED)
find_package(alpm_octopi_utils REQUIRED)
find_package(KF5Notifications QUIET)

if(NOT KF5Notifications_FOUND)
    message(WARNING "KNotifications not found. Skipping")
endif()

set(CMAKE_AUTOMOC ON)

set(src 
    main.cpp
    mainwindow.cpp
    outputdialog.cpp
    ../src/terminal.cpp
    ../src/unixcommand.cpp
    ../src/package.cpp
    ../src/wmhelper.cpp
    ../src/strconstants.cpp
    ../src/settingsmanager.cpp
    ../src/utils.cpp
    ../src/transactiondialog.cpp
    ../src/argumentlist.cpp
    ../src/pacmanexec.cpp
    ../src/searchlineedit.cpp
    ../src/searchbar.cpp
    ../src/optionsdialog.cpp
    ../src/termwidget.cpp
    ../src/aurvote.cpp
    ../src/qaesencryption.cpp
    ../src/alpmbackend.cpp)

set(header 
    mainwindow.h
    outputdialog.h
    ../src/uihelper.h
    ../src/terminal.h
    ../src/unixcommand.h
    ../src/wmhelper.h
    ../src/strconstants.h
    ../src/package.h
    ../src/utils.h
    ../src/transactiondialog.h
    ../src/argumentlist.h
    ../src/pacmanexec.h
    ../src/searchlineedit.h
    ../src/searchbar.h
    ../src/optionsdialog.h
    ../src/termwidget.h
    ../src/aurvote.h
    ../src/qaesencryption.h
    ../src/alpmbackend.h)

set(ui ../ui/transactiondialog.ui ../ui/optionsdialog.ui)

set(qrc ../resources.qrc)

qt5_wrap_ui(src ${ui})
qt5_add_resources(src ${qrc})

add_executable(octopi-notifier ${src} ${header})
target_compile_definitions(octopi-notifier PRIVATE ALPM_BACKEND QT_USE_QSTRINGBUILDER QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_URL_CAST_FROM_STRING QT_NO_CAST_FROM_BYTEARRAY)
target_include_directories(octopi-notifier PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
target_link_libraries(octopi-notifier PRIVATE Qt5::Core Qt5::Xml Qt5::Gui Qt5::Widgets Qt5::Network qtermwidget5 alpm_octopi_utils)
if(KF5Notifications_FOUND)
    target_compile_definitions(octopi-notifier PRIVATE KSTATUS)
    find_package(Qt5 REQUIRED COMPONENTS DBus)
    target_include_directories(octopi-notifier PRIVATE ${Qt5DBus_INCLUDE_DIRS})
    target_link_libraries(octopi-notifier PRIVATE Qt5::DBus KF5::Notifications)
endif()
install(TARGETS octopi-notifier RUNTIME DESTINATION bin LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/octopi-notifier.desktop" DESTINATION share/applications)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/octopi-notifier.desktop" DESTINATION /etc/xdg/autostart)
