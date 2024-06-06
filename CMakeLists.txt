cmake_minimum_required(VERSION 3.5)
project(octopi VERSION 0.16.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_THREAD_PREFER_PTHREAD True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

option(USE_QTERMWIDGET6 "Build with qtermwidget6 instead of qtermwidget5" ON)

add_subdirectory(helper)
add_subdirectory(notifier)
add_subdirectory(cachecleaner)
add_subdirectory(repoeditor)

if (USE_QTERMWIDGET6)
  find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat Gui Network Xml Widgets LinguistTools)
  find_package(qtermwidget6 REQUIRED)
else()
  find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Xml Widgets LinguistTools)
  find_package(qtermwidget5 REQUIRED)
endif()

find_package(alpm_octopi_utils REQUIRED)

set(CMAKE_AUTOMOC ON)

file(GLOB TS_FILES LIST_DIRECTORIES false "${CMAKE_CURRENT_LIST_DIR}/resources/translations/*.ts")
qt_add_translation(qmFiles ${TS_FILES})

set(src
    src/QtSolutions/qtsingleapplication.cpp
    src/QtSolutions/qtlocalpeer.cpp
    repoeditor/repoentry.cpp
    src/aurvote.cpp
    src/propertiestabwidget.cpp
    src/qaesencryption.cpp
    src/repoconf.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/strconstants.cpp
    src/searchlineedit.cpp
    src/argumentlist.cpp
    src/settingsmanager.cpp
    src/package.cpp
    src/unixcommand.cpp
    src/wmhelper.cpp
    src/treeviewpackagesitemdelegate.cpp
    src/mainwindow_init.cpp
    src/mainwindow_transaction.cpp
    src/mainwindow_events.cpp
    src/mainwindow_help.cpp
    src/searchbar.cpp
    src/mainwindow_searchbar.cpp
    src/transactiondialog.cpp
    src/mainwindow_news.cpp
    src/mainwindow_refresh.cpp
    src/globals.cpp
    src/multiselectiondialog.cpp
    src/packagerepository.cpp
    src/model/packagemodel.cpp
    src/ui/octopitabinfo.cpp
    src/utils.cpp
    src/terminal.cpp
    src/pacmanexec.cpp
    src/optionsdialog.cpp
    src/packagetreeview.cpp
    src/termwidget.cpp
    src/alpmbackend.cpp)

set(header
    src/QtSolutions/qtsingleapplication.h
    src/QtSolutions/qtlocalpeer.h
    repoeditor/repoentry.h
    src/aurvote.h
    src/propertiestabwidget.h
    src/qaesencryption.h
    src/repoconf.h
    src/mainwindow.h
    src/strconstants.h
    src/searchlineedit.h
    src/argumentlist.h
    src/settingsmanager.h
    src/uihelper.h
    src/package.h
    src/unixcommand.h
    src/wmhelper.h
    src/treeviewpackagesitemdelegate.h
    src/searchbar.h
    src/transactiondialog.h
    src/globals.h
    src/multiselectiondialog.h
    src/packagerepository.h
    src/model/packagemodel.h
    src/ui/octopitabinfo.h
    src/utils.h
    src/terminal.h
    src/pacmanexec.h
    src/constants.h
    src/optionsdialog.h
    src/packagetreeview.h
    src/termwidget.h
    src/alpmbackend.h)

set(ui ui/mainwindow.ui ui/transactiondialog.ui ui/multiselectiondialog.ui ui/optionsdialog.ui)

set(qrc resources.qrc)

qt_wrap_ui(src ${ui})
qt_add_resources(src ${qrc})

add_executable(octopi ${src} ${header} ${qmFiles})
target_compile_definitions(octopi PRIVATE OCTOPI_EXTENSIONS ALPM_BACKEND QT_DEPRECATED_WARNINGS QT_USE_QSTRINGBUILDER QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_URL_CAST_FROM_STRING QT_NO_CAST_FROM_BYTEARRAY)

if (USE_QTERMWIDGET6)
  target_include_directories(octopi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Qt6Core_INCLUDE_DIRS} ${Qt6Gui_INCLUDE_DIRS} ${Qt6Network_INCLUDE_DIRS} ${Qt6Xml_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS})
  target_link_libraries(octopi PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Xml Qt6::Widgets qtermwidget6 alpm_octopi_utils)
else()
  target_include_directories(octopi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
  target_link_libraries(octopi PRIVATE Qt5::Core Qt5::Gui Qt5::Network Qt5::Xml Qt5::Widgets qtermwidget5 alpm_octopi_utils)
endif()

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources/images/octopi_green.png" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(RENAME "${CMAKE_CURRENT_BINARY_DIR}/octopi_green.png" "${CMAKE_CURRENT_BINARY_DIR}/octopi.png")
install(TARGETS octopi RUNTIME DESTINATION bin LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/octopi.desktop" DESTINATION share/applications)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/octopi.png" "${CMAKE_CURRENT_SOURCE_DIR}/resources/images/octopi_green.png" DESTINATION share/icons/gnome/32x32/apps)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/octopi.png" "${CMAKE_CURRENT_SOURCE_DIR}/resources/images/octopi_green.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/images/octopi_red.png" "${CMAKE_CURRENT_SOURCE_DIR}/resources/images/octopi_yellow.png" DESTINATION share/icons)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" DESTINATION share/licenses/octopi)
