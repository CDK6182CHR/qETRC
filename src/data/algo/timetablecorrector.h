#pragma once

#include <memory>

class Train;
/**
 * @brief The TimetableCorrector class
 * 2022.05.27  列车时刻表修正算法
 * 暂时维护成Static函数的形式；暂时不需要有状态。
 * Timetable_new.checi3.Train.dectectError()
 * 现有实现直接Copy Python版本，不保证内容地址不变。
 */
class TimetableCorrector
{
public:
    TimetableCorrector()=default;

    /**
     * @brief autoCorrect
     * 主函数：返回是否有修改。
     */
    static bool autoCorrect(std::shared_ptr<Train> train);

private:

    /**
     * 2022.05.27
     * 对应Timetable_new.checi3.Train.detectError()
     * 一次查错循环，返回是否修改了时刻表。
     * 暂定直接翻译Python代码
     */
    static bool correctCycle(std::shared_ptr<Train> train);

    /**
     * 判定是否是相邻区间。要比较保守的判定（不确定倾向于否）。阈值暂定30分钟。
     */
    static bool neighbourInterval(int secs);

    static bool errorInterval(int secs);
};

