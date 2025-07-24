#pragma once

#include <QAbstractSpinBox>
#include <QRegularExpression>

#include "data/common/traintime.h"

/**
 * @brief The TrainTimeEdit class
 * Edit the TrainTime class using a spin box similar to QTimeEdit.
 * With private part unavailable, some implementations are different from QTimeEdit,
 * and some data may be duplicate...
 */
class TrainTimeEdit : public QAbstractSpinBox
{
    Q_OBJECT
    TrainTime m_time = TrainTime::fromSecondsSinceStart(0);
    int m_maxHours = 24;
    TrainTime::TimeFormat m_format = TrainTime::HMS;
    QRegularExpression m_regex;

public:
    enum Section { HourSection = 0, MinuteSection, SecondSection };

    TrainTimeEdit(QWidget* parent = nullptr);

    Section currentSection()const;

    auto format()const { return m_format; }
    void setFormat(TrainTime::TimeFormat f);

    QValidator::State validate(QString& input, int& pos) const override;

    void stepBy(int steps)override;

    int maxHours()const { return m_maxHours; }
    void setMaxHours(int h) { m_maxHours = h; }

    /**
     * @brief stepEnabled
     * Currently only impl from DeepSeek, we have not verified this!!
     */
    StepEnabled stepEnabled()const override;

    void setTime(const TrainTime& tm);
    auto& time()const { return m_time; }

    QSize sizeHint()const override;

protected:
    bool focusNextPrevChild(bool next)override;

    void focusInEvent(QFocusEvent* e)override;

    void focusOutEvent(QFocusEvent* e)override;

private:
    Section firstSection()const;
    Section lastSection()const;

    Section positionToSection(int pos)const;

    /**
     * Tells that whether the current position is at the end of current section,
     * with no more possible digits.
     * If so, we should move to next section.
     * But if current section is the last section, just return false,
     * because we do not automatically focus on the next widget.
     */
    bool atEndOfNonLastSection(int pos)const;

signals:
    void timeChanged(const TrainTime& time);

public slots:
    void updateTime();

private slots:
    void onTextChanged();

    void selectCurrentSection();

    /**
     * Move cursor to, and select the required section.
     */
    void selectSection(Section section);
};

