﻿#include "version.h"
#include "version_predef.h"   // generated by CMake

namespace qespec {

const std::string_view VERSION = QETRC_VERSION;
const std::string_view LAST_VERSION = "V1.7.5";
const std::string_view DATE = "20241208";
const int RELEASE_CODE = 59;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 新增自动上下边距功能。
2. 允许在速览时刻/速览信息窗口中查阅正在铺画列车的时刻及信息。
3. 更新支持库版本，及细节修复或优化。)";
}
