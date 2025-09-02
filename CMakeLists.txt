cmake_minimum_required(VERSION 3.16)

project(SalivaryGlandsViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# --- ITK ---
find_package(ITK REQUIRED COMPONENTS ITKVTK ITKIOGDCM ITKIOImageBase)
set(VTK_DIR "/Users/greg/repo/ITK/build/lib/cmake/ITK-5.4/Modules")

# --- VTK ---
set(VTK_DIR "/Users/greg/repo/vtk/build/lib/cmake/vtk-9.5")
find_package(VTK REQUIRED)

set(TS_FILES SalivaryGlandsViewer_en_AS.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        include/mainwindow.hpp
        ui/mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SalivaryGlandsViewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}


    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(SalivaryGlandsViewer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SalivaryGlandsViewer
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(SalivaryGlandsViewer PRIVATE
    Qt6::Widgets
    ${ITK_LIBRARIES}
    ${VTK_LIBRARIES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SalivaryGlandsViewer)
endif()
set_target_properties(SalivaryGlandsViewer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SalivaryGlandsViewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SalivaryGlandsViewer)
endif()
