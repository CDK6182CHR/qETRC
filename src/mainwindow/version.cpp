#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.2.3";
const std::string_view LAST_VERSION = "V1.2.2";
const std::string_view DATE = "20221029";
const int RELEASE_CODE = 30;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 在列车时刻表修正功能中新增局部严格排序功能。
2. 新增标尺编辑中批量设置起停附加时分。
3. 修复标尺排图提交空数据时崩溃的问题。)";
}
