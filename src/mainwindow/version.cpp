#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.1.12";
const std::string_view LAST_VERSION = "V1.1.11";
const std::string_view DATE = "20220828";
const int RELEASE_CODE = 26;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 修复某些极端情况下，运行线铺画逻辑错误的问题。
2. 新增内置在线文档链接和启动提示页。
)";
}
