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
# 2024.03.28: use RelWithDebInfo build
BUILD_DIR = ..\out\install\x64-RD\bin
BIN_FILE = qETRC.exe
PDB_FILE = qETRC.pdb
QT_PROMPT_SCRIPT = D:\programs\Qt\6.5.1\msvc2019_64\bin\qtenv2.bat

# specify output dir
OUTPUT_ROOT = $(QETRC_ROOT)\..\archive
WIN64_DIR = $(OUTPUT_ROOT)\win64
PATCH_DIR = $(OUTPUT_ROOT)\patch

WIN64_ARCHIVE = qETRC-win64$(SUFFIX).7z
PATCH_ARCHIVE = qETRC-win64-patch$(SUFFIX).7z
PDB_ARCHIVE = qETRC-pdb-archive$(SUFFIX).7z


SARIBBON_LIB = $(BUILD_DIR)\SARibbonBarrd.dll 
QWK_CORE_LIB = $(BUILD_DIR)\QWKCorerd.dll 
QWK_WIDGETS_LIB = $(BUILD_DIR)\QWKWidgetsrd.dll
ADS_LIB = $(BUILD_DIR)\qt5advanceddocking.dll

# Other files
NOTE_FILE = ..\ReleaseNote.pdf
SAMPLE_FILE = ..\sample.pyetgr
RAIL_LIB_FILE = ..\CRPassengerMileage.pyetlib

# command alias
COPY = copy
XCOPY = xcopy
ZIP = 7z a

.PHONY: all
all: win64 win64-patch pdb-archive


# This target generates win64 archive
.PHONY: win64
win64: win64_dir win64_tr
	$(COPY) $(BUILD_DIR)\$(BIN_FILE) $(WIN64_DIR)
	$(COPY) $(NOTE_FILE) $(WIN64_DIR)
	$(COPY) $(SAMPLE_FILE) $(WIN64_DIR)
	$(COPY) $(RAIL_LIB_FILE) $(WIN64_DIR)
	$(COPY) $(SARIBBON_LIB) $(WIN64_DIR)
	$(COPY) $(QWK_CORE_LIB) $(WIN64_DIR)
	$(COPY) $(QWK_WIDGETS_LIB) $(WIN64_DIR)
	$(COPY) $(QETRC_ROOT)\tr\*.qm $(WIN64_DIR)\tr
	$(QT_PROMPT_SCRIPT) && cd /d $(WIN64_DIR) && cd && windeployqt $(BIN_FILE)
	cd $(OUTPUT_ROOT) && $(ZIP) $(WIN64_ARCHIVE) $(WIN64_DIR)


# This target for win64-patch
.PHONY: win64-patch
win64-patch: patch_dir patch_tr
	$(COPY) $(BUILD_DIR)\$(BIN_FILE) $(PATCH_DIR)
	$(COPY) $(NOTE_FILE) $(PATCH_DIR)
	$(COPY) $(QETRC_ROOT)\tr\*.qm $(PATCH_DIR)\tr
	cd $(OUTPUT_ROOT) && $(ZIP) $(PATCH_ARCHIVE) $(PATCH_DIR)


# 2024.03.28: pdb archive for future debugging
PDB_DIR = $(OUTPUT_ROOT)\pdb-archive
.PHONY: pdb-archive
pdb-archive: pdb_dir
	$(COPY) $(BUILD_DIR)\$(BIN_FILE) $(PDB_DIR)
	$(COPY) $(BUILD_DIR)\$(PDB_FILE) $(PDB_DIR)
	cd $(OUTPUT_ROOT) && $(ZIP) $(PDB_ARCHIVE) $(PDB_DIR)
	
# This is for local debug version
LOCAL_DIR = $(QETRC_ROOT)\..\local

.PHONY: local
local:
	$(COPY) $(BUILD_DIR)\$(BIN_FILE) $(LOCAL_DIR)
	$(COPY) $(SARIBBON_LIB) $(LOCAL_DIR)
	$(COPY) $(QWK_CORE_LIB) $(LOCAL_DIR)
	$(COPY) $(QWK_WIDGETS_LIB) $(LOCAL_DIR)
	-mkdir $(LOCAL_DIR)\tr 
	$(COPY) $(QETRC_ROOT)\tr\*.qm $(LOCAL_DIR)\tr\

win64_dir:
	-mkdir $(WIN64_DIR)

patch_dir:
	-mkdir $(PATCH_DIR)

win64_tr: win64_dir
	-mkdir $(WIN64_DIR)\tr 

patch_tr: patch_dir
	-mkdir $(PATCH_DIR)\tr

pdb_dir: 
	-mkdir $(PDB_DIR)