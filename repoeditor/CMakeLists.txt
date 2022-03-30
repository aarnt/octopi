find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Xml Widgets LinguistTools)
find_package(Threads REQUIRED)
find_library(UTIL_LIBRARY NAMES util)

set(CMAKE_AUTOMOC ON)

file(GLOB TS_FILES LIST_DIRECTORIES false "${CMAKE_CURRENT_LIST_DIR}/resources/translations/*.ts")
qt5_add_translation(qmFiles ${TS_FILES})

set(src
    addrepo.cpp
    checkboxdelegate.cpp
    main.cpp
    optionsdelegate.cpp
    repoconf.cpp
    repoeditor.cpp
    repoentry.cpp
    ../src/qaesencryption.cpp
    ../src/unixcommand.cpp
    ../src/strconstants.cpp
    ../src/wmhelper.cpp
    ../src/terminal.cpp
    ../src/settingsmanager.cpp
    ../src/searchlineedit.cpp
    ../src/utils.cpp
    ../src/package.cpp
    ../src/QtSolutions/qtsingleapplication.cpp
    ../src/QtSolutions/qtlocalpeer.cpp
    #../src/QtSolutions/qtlockedfile.cpp
    ../src/QtSolutions/qtsinglecoreapplication.cpp)

set(header
    addrepo.h
    checkboxdelegate.h
    optionsdelegate.h
    repoconf.h
    repoeditor.h
    repoentry.h
    ../src/qaesencryption.h
    ../src/unixcommand.h
    ../src/strconstants.h
    ../src/wmhelper.h
    ../src/terminal.h
    ../src/settingsmanager.h
    ../src/searchlineedit.h
    ../src/utils.h
    ../src/package.h
    ../src/QtSolutions/qtsingleapplication.h
    ../src/QtSolutions/qtlocalpeer.h
    #../src/QtSolutions/qtlockedfile.h
    ../src/QtSolutions/qtsinglecoreapplication.h)

set(ui addrepo.ui repoeditor.ui)

set(qrc resources.qrc)

qt5_wrap_ui(src ${ui})
qt5_add_resources(src ${qrc})

add_executable(octopi-repoeditor ${src} ${header} ${qmFiles})
target_compile_definitions(octopi-repoeditor PRIVATE QT_USE_QSTRINGBUILDER QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_URL_CAST_FROM_STRING QT_NO_CAST_FROM_BYTEARRAY)
target_include_directories(octopi-repoeditor PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
target_link_libraries(octopi-repoeditor PRIVATE Threads::Threads Qt5::Core Qt5::Gui Qt5::Network Qt5::Xml Qt5::Widgets ${UTIL_LIBRARY})
install(TARGETS octopi-repoeditor RUNTIME DESTINATION bin LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/octopi-repoeditor.desktop" DESTINATION share/applications)
