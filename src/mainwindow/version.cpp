#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.6.1";
const std::string_view LAST_VERSION = "V1.6.0";
const std::string_view DATE = "20240317";
const int RELEASE_CODE = 50;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 新增批量设置列车运行线样式功能。
2. 修复存在单向站时，导出ETRC运行图的问题。)";
}
