cmake_minimum_required(VERSION 3.14)

include(qETRC_version.cmake)

option(ENABLE_MSVC_PROFILE "Enable MSVC profiler" OFF)
option(AUTO_INSTALL_SARibbon "Install SARibbon libraries" ON)
option(AUTO_INSTALL_QWK "Install QWindowKit libraries, dependent by SARibbon" ON)
option(QETRC_EXTRA_WARNING_INFO "use extra warning messages" ON)

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

configure_file(
    version_predef.h.in
    version_predef.h
    @ONLY
)

file(GLOB_RECURSE qETRC_SOURCES "src/*.cpp")
file(GLOB_RECURSE qETRC_HEADERS "src/*.h")
file(GLOB_RECURSE qETRC_HEADERS_PP "src/*.hpp")

if (MSVC)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
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

    # 2024.03.28 trial: use find_package for SARibbon
    # add_subdirectory(external/SARibbon)
    find_package(SARibbonBar REQUIRED)
    message(STATUS "Found SARibbonBar version ${SARibbonBar_VERSION}")
    
    # list(APPEND CMAKE_PREFIX_PATH "D:/codes/QtProjects/site-packages/Qt-Advanced-Docking-System/out/install/lib/cmake/qt5advanceddocking")
    # find_package(qt${QT_VERSION_MAJOR}advanceddocking CONFIG REQUIRED)

    if (NOT ADS_REPO_URL)
        set(ADS_REPO_URL https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System)
    endif ()

    # try: using FetchContent
    FetchContent_Declare(
        ads
        # GIT_REPOSITORY ${CMAKE_SOURCE_DIR}/external/Qt-Advanced-Docking-System
        GIT_REPOSITORY ${ADS_REPO_URL}
        GIT_TAG 4.4.0
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ads
    )
    FetchContent_MakeAvailable(ads)

    target_link_libraries(qETRC PUBLIC SARibbonBar::SARibbonBar)
    target_link_libraries(qETRC PRIVATE qtadvanceddocking-qt${QT_VERSION_MAJOR})

    if (AUTO_INSTALL_SARibbon)
        get_target_property(SARibbonBar_LIB_FILE SARibbonBar::SARibbonBar LOCATION_${CMAKE_BUILD_TYPE})
        message(STATUS "SARibbon lib ${SARibbonBar_LIB_FILE}")
        install(FILES ${SARibbonBar_LIB_FILE} DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif (AUTO_INSTALL_SARibbon)

    if (AUTO_INSTALL_QWK) 
        if (NOT QWindowKit_DIR)
            set (QWindowKit_DIR ${SARibbonBar_DIR}/../QWindowKit)
        endif ()

        find_package(QWindowKit CONFIG)
        if (QWindowKit_FOUND)
            get_target_property(QWK_CORE_FILE QWindowKit::Core LOCATION_${CMAKE_BUILD_TYPE})
            get_target_property(QWK_WIDGETS_FILE QWindowKit::Widgets LOCATION_${CMAKE_BUILD_TYPE})
            message(STATUS "QWK lib ${QWK_CORE_FILE}")
            install(FILES ${QWK_CORE_FILE} ${QWK_WIDGETS_FILE} DESTINATION ${CMAKE_INSTALL_BINDIR})
        else ()
            message(STATUS "QWK not found, install not performed")
        endif ()
    endif (AUTO_INSTALL_QWK)
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
        #SET_TARGET_PROPERTIES(ads::qt${QT_VERSION_MAJOR}advanceddocking PROPERTIES LINK_FLAGS "/PROFILE")
        SET_TARGET_PROPERTIES(qETRC PROPERTIES LINK_FLAGS "/PROFILE")
    endif ()

endif()

if (QETRC_EXTRA_WARNING_INFO)
    if (MSVC)
        # Remove the automatically configured /W3 flag, as this would generate command-line warning...
        string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

        # C4458: declaration overwrites class member. This is rather common in this project, so disable this for now..
        target_compile_options(qETRC PRIVATE /W4 /wd4458)
    else ()
        target_compile_options(qETRC PRIVATE -Wall -Wextra)
    endif ()
endif ()

message(STATUS "Flags: ${CMAKE_CXX_FLAGS} -- ${CMAKE_CXX_FLAGS_INIT} -- ${CMAKE_CXX_FLAGS_DEBUG} -- ${CMAKE_CXX_FLAGS_RELEASE}")

if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
    message("disable console for Release buildings.")
    # disable console for release version
    set_property(TARGET qETRC PROPERTY WIN32_EXECUTABLE true)
endif()

if(WIN32)
    include(cmake/WinResource.cmake)
	create_win32_resource_version(
                TARGET qETRC
                FILENAME qETRC
		EXT "exe"
		DESCRIPTION "qETRC train working diagram system."
	)
endif()

# translations; seems only applicable for Qt6
if (QT_VERSION_MAJOR GREATER 5)
    qt_add_lupdate(qETRC TS_FILES tr/qETRC_zh.ts tr/qETRC_en.ts)
    qt_add_lrelease(qETRC TS_FILES tr/qETRC_zh.ts tr/qETRC_en.ts)
endif ()

install(TARGETS qETRC
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install PDB
if (MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> 
    DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qETRC)
endif()



