﻿#include "version.h"
#include "version_predef.h"   // generated by CMake

namespace qespec {

const std::string_view VERSION = QETRC_VERSION;
const std::string_view LAST_VERSION = "V1.7.9";
const std::string_view DATE = "20250220";
const int RELEASE_CODE = 63;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(修复输出PDF/PNG运行图时字体尺寸不正确的问题。)";
}
