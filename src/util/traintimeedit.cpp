#include "traintimeedit.h"

#include <QLineEdit>
#include <QFocusEvent>

TrainTimeEdit::TrainTimeEdit(QWidget* parent) :
    QAbstractSpinBox(parent)
{
    updateTime();
    setFormat(TrainTime::HMS);
    connect(lineEdit(), &QLineEdit::textChanged,
        this, &TrainTimeEdit::onTextChanged);
}

void TrainTimeEdit::setFormat(TrainTime::TimeFormat f)
{
    m_format = f;
    switch (f) {
    case TrainTime::HMS: m_regex = QRegularExpression("^(\\d+):(\\d{1,2}):(\\d{1,2})$"); break;
    case TrainTime::HM: m_regex = QRegularExpression("^(\\d+):(\\d{1,2})$"); break;
    }
}

QValidator::State TrainTimeEdit::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos)
        auto res = m_regex.match(input);
    if (!res.hasMatch()) return QValidator::Invalid;

    bool ok;
    int h = res.capturedView(1).toInt(&ok);
    if (!ok) return QValidator::Invalid;
    int m = res.capturedView(2).toInt(&ok);
    if (!ok) return QValidator::Invalid;
    int s = 0;
    if (m_format == TrainTime::HMS) {
        s = res.capturedView(3).toInt(&ok);
        if (!ok) return QValidator::Invalid;
    }

    return (h >= 0 && h < m_maxHours && m >= 0 && m < 60 && s >= 0 && s < 60) ?
        QValidator::Acceptable : QValidator::Invalid;
}

void TrainTimeEdit::stepBy(int steps)
{
    auto sec = currentSection();
    switch (currentSection()) {
    case HourSection:   m_time.addHoursInplace(steps);   break;
    case MinuteSection: m_time.addMinutesInplace(steps); break;
    case SecondSection: m_time.addSecondsInplace(steps); break;
    }
    m_time.refineHour(m_maxHours);
    updateTime();

    // We cannot use selectCurrentSection() here, because the section indicated by the
    // cursor may changed after stepping (if the number of digits changed, etc)
    selectSection(sec);
    emit timeChanged(m_time);
}

bool TrainTimeEdit::focusNextPrevChild(bool next)
{
    auto sec = currentSection();
    if (next) {
        if (sec == SecondSection || (m_format == TrainTime::HM && sec == MinuteSection))
            return QAbstractSpinBox::focusNextPrevChild(next);   // Find next widget
        else {
            selectSection(static_cast<Section>(sec + 1));
            return true;
        }
    }
    else {
        // backward
        if (sec == HourSection) {
            return QAbstractSpinBox::focusNextPrevChild(next);
        }
        else {
            selectSection(static_cast<Section>(sec - 1));
            return true;
        }
    }
}

void TrainTimeEdit::focusInEvent(QFocusEvent* e)
{
    QAbstractSpinBox::focusInEvent(e);
    if (e->reason() == Qt::TabFocusReason)
        selectSection(firstSection());
    else if (e->reason() == Qt::BacktabFocusReason)
        selectSection(lastSection());
}

TrainTimeEdit::Section TrainTimeEdit::lastSection() const
{
    switch (m_format) {
    case TrainTime::HMS: return SecondSection;
    case TrainTime::HM: return MinuteSection;
    default:
        qWarning() << "Invalid format";
        return HourSection;
    }
}

TrainTimeEdit::Section TrainTimeEdit::firstSection() const
{
    return HourSection;
}

QAbstractSpinBox::StepEnabled TrainTimeEdit::stepEnabled() const
{
    // 2025.03.19: currently, always enable step
    return StepUpEnabled | StepDownEnabled;
    // const bool up = lineEdit()->cursorPosition() < 8;
    // const bool down = lineEdit()->cursorPosition() > 0;

    // return StepEnabled((up ? StepUpEnabled : 0) | (down ? StepDownEnabled : 0));
}

void TrainTimeEdit::setTime(const TrainTime& tm)
{
    if (tm != m_time) {
        m_time = tm;
        updateTime();
        emit timeChanged(m_time);
    }
}

void TrainTimeEdit::updateTime()
{
    int currrent_cursor_pos = lineEdit()->cursorPosition();
    const QString old = lineEdit()->text();
    lineEdit()->setText(m_time.toString(m_format));
    if (old != lineEdit()->text())
        lineEdit()->setCursorPosition(qMin(currrent_cursor_pos, lineEdit()->text().length()));
}

void TrainTimeEdit::onTextChanged()
{
    QString t = lineEdit()->text();
    int p = 0;
    auto stat = validate(t, p);
    if (stat == QValidator::Acceptable) {
        // Indicate the input is valid
        m_time = TrainTime::fromString(t);
    }
    updateTime();
}

void TrainTimeEdit::selectCurrentSection()
{
    const QString& txt = lineEdit()->text();
    int pos = lineEdit()->cursorPosition();
    int next_delim = txt.indexOf(':', pos);
    if (next_delim < 0) next_delim = txt.size();
    int last_delim = pos == 0 ? -1 : txt.lastIndexOf(':', pos - 1);
    // if (last_delim < 0) last_delim = 0;   // -1 is automatically converted to 0 later
    lineEdit()->setSelection(last_delim + 1, next_delim - last_delim - 1);
}

void TrainTimeEdit::selectSection(Section section)
{
    int sec_idx = static_cast<int>(section);
    const QString& txt = lineEdit()->text();
    int start_idx = 0;
    for (int i = 0; i < sec_idx; i++) {
        start_idx = txt.indexOf(':', start_idx + 1);
    }

    if (start_idx < 0) {
        qDebug() << "TrainTimeEdit::selectSection: required section not found";
        return;
    }
    if (start_idx > 0) start_idx += 1;   // Selection starts from the char NEXT TO colon

    int end_idx = txt.indexOf(':', start_idx + 1);
    if (end_idx < 0) end_idx = txt.size();

    lineEdit()->setCursorPosition(end_idx - 1);
    lineEdit()->setSelection(start_idx, end_idx - start_idx);
}

TrainTimeEdit::Section TrainTimeEdit::currentSection() const
{
    int pos = lineEdit()->cursorPosition();
    int tot_len = lineEdit()->text().length();

    if (m_format == TrainTime::HMS) {
        if (tot_len - pos < 2) {
            return SecondSection;
        }
        else if (tot_len - pos < 5) {
            return MinuteSection;
        }
        else {
            return HourSection;
        }
    }
    else {
        // HM format
        if (tot_len - pos < 2) {
            return MinuteSection;
        }
        else {
            return HourSection;
        }
    }
}
