#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.7.0";
const std::string_view LAST_VERSION = "V1.6.3";
const std::string_view DATE = "20240405";
const int RELEASE_CODE = 53;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 升级依赖库版本。
2. 新增列车审阅页面中添加/创建交路。
3. 增强拖动调整时刻功能，以支持部分运行线平移。
4. 新增显示停点的秒数修约选项和其他细节。)";
}
