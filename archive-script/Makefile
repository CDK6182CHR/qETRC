# NOTE: This Makefile is for generating release package, 
# not for build project. 
# To build project, please use qmake

# This variable should be passed from command-line
# to be used as suffix of archive 7z file name.
SUFFIX = 

# The root dir for qETRC project, which contains `src`
# The Makefile is assumed to be located in $(QETRC_ROOT)\release-script
QETRC_ROOT = D:\codes\QtProjects\qETRC\qETRC

# use MSVC build 
BUILD_DIR = ..\out\install\x64-Release\bin
BIN_FILE = qETRC.exe
QT_PROMPT_SCRIPT = D:\programs\Qt\6.5.1\msvc2019_64\bin\qtenv2.bat

# specify output dir
OUTPUT_ROOT = $(QETRC_ROOT)\..\archive
WIN64_DIR = $(OUTPUT_ROOT)\win64
PATCH_DIR = $(OUTPUT_ROOT)\patch

WIN64_ARCHIVE = qETRC-win64$(SUFFIX).7z
PATCH_ARCHIVE = qETRC-win64-patch$(SUFFIX).7z

# dependency lib
ARC_NAME = Qt5.15.2-win32-msvc
LIB_ROOT = ..\lib

SARIBBON_LIB = $(BUILD_DIR)\SARibbonBar.dll
ADS_LIB = $(BUILD_DIR)\qt5advanceddocking.dll

# Other files
NOTE_FILE = ..\ReleaseNote.pdf
SAMPLE_FILE = ..\sample.pyetgr
RAIL_LIB_FILE = ..\CRPassengerMileage.pyetlib

# command alias
COPY = copy
ZIP = 7z a

.PHONY: all
all: win64 win64-patch


# This target generates win64 archive
.PHONY: win64
win64: win64_dir
	$(COPY) $(BUILD_DIR)\$(BIN_FILE) $(WIN64_DIR)
	$(COPY) $(NOTE_FILE) $(WIN64_DIR)
	$(COPY) $(SAMPLE_FILE) $(WIN64_DIR)
	$(COPY) $(RAIL_LIB_FILE) $(WIN64_DIR)
	$(COPY) $(SARIBBON_LIB) $(WIN64_DIR)
	$(QT_PROMPT_SCRIPT) && cd /d $(WIN64_DIR) && cd && windeployqt $(BIN_FILE)
	cd $(OUTPUT_ROOT) && $(ZIP) $(WIN64_ARCHIVE) $(WIN64_DIR)


# This target for win64-patch
.PHONY: win64-patch
win64-patch: patch_dir
	$(COPY) $(BUILD_DIR)\$(BIN_FILE) $(PATCH_DIR)
	$(COPY) $(NOTE_FILE) $(PATCH_DIR)
	cd $(OUTPUT_ROOT) && $(ZIP) $(PATCH_ARCHIVE) $(PATCH_DIR)
	
# This is for local debug version
LOCAL_DIR = $(QETRC_ROOT)\..\local

.PHONY: local
local:
	$(COPY) $(BUILD_DIR)\$(BIN_FILE) $(LOCAL_DIR)
	$(COPY) $(SARIBBON_LIB) $(LOCAL_DIR)

win64_dir:
	-mkdir $(WIN64_DIR)

patch_dir:
	-mkdir $(PATCH_DIR)