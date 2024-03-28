cmake_minimum_required(VERSION 3.14)

set(qETRC_VERSION_MAJOR 1)
set(qETRC_VERSION_MINOR 6)
set(qETRC_VERSION_PATCH 3)
#set(qETRC_RELEASE_CODE 40)

option(ENABLE_MSVC_PROFILE "Enable MSVC profiler" OFF)

set(qETRC_VERSION "${qETRC_VERSION_MAJOR}.${qETRC_VERSION_MINOR}.${qETRC_VERSION_PATCH}")

message(STATUS "qETRC version ${qETRC_VERSION}")

project(qETRC LANGUAGES CXX VERSION ${qETRC_VERSION})


################# Qt basic ################
#list(PREPEND CMAKE_PREFIX_PATH $ENV{QTDIR}/lib/cmake)
#message(STATUS "env QTDIR $ENV{QTDIR}")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} 5.11 COMPONENTS Core Gui Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.11 COMPONENTS PrintSupport)

################## main ##################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE qETRC_SOURCES "src/*.cpp")
file(GLOB_RECURSE qETRC_HEADERS "src/*.h")
file(GLOB_RECURSE qETRC_HEADERS_PP "src/*.hpp")

if (NOT ANDROID)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/qETRC_resource.rc")
endif()

# add_executable(qETRC
#     ${qETRC_SOURCES}
#     rsc/resource.qrc
#     ${app_icon_resource_windows}
# )
set(PROJECT_SOURCES ${qETRC_SOURCES} ${qETRC_HEADERS} ${qETRC_HEADERS_PP}
    rsc/resource.qrc ${app_icon_resource_windows})


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qETRC
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET QtAndroidCMakeTest APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qETRC SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
   set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Android_source")
    else()
        add_executable(qETRC
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(qETRC PRIVATE src/)

target_link_libraries(qETRC PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if (Qt${QT_VERSION_MAJOR}PrintSupport_FOUND)
    message(STATUS "QtPrintSupport is found")
    target_link_libraries(qETRC PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)
else()
    message(WARNING "QtPrintSupport not found")
endif()


################# external libraries ##################
include(FetchContent)

if (NOT ANDROID)
    # disable example building in SARibbon, since some of them requires Qt6 ..
    set(BUILD_EXAMPLES OFF)

    add_subdirectory(external/SARibbon)
    
    # list(APPEND CMAKE_PREFIX_PATH "D:/codes/QtProjects/site-packages/Qt-Advanced-Docking-System/out/install/lib/cmake/qt5advanceddocking")
    # find_package(qt${QT_VERSION_MAJOR}advanceddocking CONFIG REQUIRED)

    # try: using FetchContent
    FetchContent_Declare(
        ads
        # GIT_REPOSITORY ${CMAKE_SOURCE_DIR}/external/Qt-Advanced-Docking-System
        GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
        GIT_TAG 4.0.3
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ads
    )
    FetchContent_MakeAvailable(ads)

    target_link_libraries(qETRC PRIVATE SARibbonBar)
    target_link_libraries(qETRC PRIVATE ads::qt${QT_VERSION_MAJOR}advanceddocking)
endif()


#################### miscellaneous #####################

if (ANDROID)
    target_compile_definitions(qETRC PUBLIC QETRC_MOBILE QETRC_MOBILE_2)
endif()

if (MSVC)
    target_compile_options(qETRC PRIVATE /utf-8 /wd4267 /Zc:__cplusplus ) 

    # MSVC profiler  https://learn.microsoft.com/zh-cn/cpp/build/reference/profile-performance-tools-profiler?view=msvc-170
    if (ENABLE_MSVC_PROFILE)
        message(STATUS "MSVC profiler enabled")
        SET_TARGET_PROPERTIES(SARibbonBar PROPERTIES LINK_FLAGS "/PROFILE")
        #SET_TARGET_PROPERTIES(ads::qt${QT_VERSION_MAJOR}advanceddocking PROPERTIES LINK_FLAGS "/PROFILE")
        SET_TARGET_PROPERTIES(qETRC PROPERTIES LINK_FLAGS "/PROFILE")
    endif ()

endif()

if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message("disable console for Release buildings.")
    # disable console for release version
    set_property(TARGET qETRC PROPERTY WIN32_EXECUTABLE true)
endif()

# translations
qt_add_lupdate(qETRC TS_FILES tr/qETRC_zh.ts tr/qETRC_en.ts)
qt_add_lrelease(qETRC TS_FILES tr/qETRC_zh.ts tr/qETRC_en.ts)

install(TARGETS qETRC
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qETRC)
endif()



