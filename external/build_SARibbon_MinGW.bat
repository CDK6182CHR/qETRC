
rem echo off

rem 2024.03.28  This scripts builds the SARibbon v2.0.3 with CMake.

rem set QT_DIR=D:\programs\Qt\6.5.1\msvc2019_64\lib\cmake\Qt6

for %%c in (Debug Release RelWithDebInfo) do (
	
rem  Build QWK
cd SARibbon\src\SARibbonBar\3rdparty
if not exist out\build_MinGW\%%c  md out\build_MinGW\%%c
cd out\build_MinGW\%%c

cmake -G Ninja ../../.. -DCMAKE_BUILD_TYPE=%%c -DCMAKE_DEBUG_POSTFIX=d -DCMAKE_RELWITHDEBINFO_POSTFIX=rd -DQT_DIR=%QT_DIR%
cmake --build . --config %%c
cmake --install . --config %%c

cd ..\..\..

rem  Now, for SARibbon
cd ..\..\..
if not exist out\build_MinGW\%%c  md out\build_MinGW\%%c
cd out\build_MinGW\%%c

cmake -G Ninja ../../.. -DCMAKE_BUILD_TYPE=%%c -DQT_DIR=%QT_DIR% -DSARIBBON_USE_FRAMELESS_LIB=ON -DCMAKE_DEBUG_POSTFIX=d -DCMAKE_RELWITHDEBINFO_POSTFIX=rd
cmake --build . --config %%c
cmake --install . --config %%c

cd ..\..\..

cd ..
)
