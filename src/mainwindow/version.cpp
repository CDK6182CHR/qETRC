﻿#include "version.h"
#include "version_predef.h"   // generated by CMake

namespace qespec {

const std::string_view VERSION = QETRC_VERSION;
const std::string_view LAST_VERSION = "V1.7.6";
const std::string_view DATE = "20250115";
const int RELEASE_CODE = 60;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 修复断面事件表中，运行线跨日导致的错误。
2. 更新SARibbon版本至2.2.6。)";
}
