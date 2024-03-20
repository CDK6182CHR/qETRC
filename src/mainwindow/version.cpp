#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.6.3";
const std::string_view LAST_VERSION = "V1.6.2";
const std::string_view DATE = "20240320";
const int RELEASE_CODE = 52;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 修复列车径路相关的数据同步以及极端情况下程序崩溃问题。
2. 在使用列车径路铺画运行线时，不再允许单车站的运行线。
3. 修复某些内存泄漏问题。)";
}
