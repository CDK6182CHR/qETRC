#include "traintimeedit.h"

#include <QLineEdit>
#include <QFocusEvent>
#include <QStyleOptionSpinBox>

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
        // 2025.08.25: we accept empty sections for intermediate state while editing.
    case TrainTime::HMS: m_regex = QRegularExpression("^(\\d*):(\\d{0,2}):(\\d{0,2})$"); break;
    case TrainTime::HM: m_regex = QRegularExpression("^(\\d*):(\\d{0,2})$"); break;
    }
}

QValidator::State TrainTimeEdit::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos)
        auto res = m_regex.match(input);
    if (!res.hasMatch()) return QValidator::Invalid;

    bool ok;

    // 2025.08.25 new version: empty sections are treated as zero, and is considered
    // to be valid.
    int h = 0;
    if (const auto& rv = res.capturedView(1); !rv.empty()) {
        h = rv.toInt(&ok);
        if (!ok) return QValidator::Invalid;
    }

    int m = 0;
    if (const auto& rv = res.capturedView(2); !rv.empty()) {
        m = rv.toInt(&ok);
        if (!ok) return QValidator::Invalid;
    }

    int s = 0;
    if (m_format == TrainTime::HMS) {
        if (const auto& rv = res.capturedView(3); !rv.empty()) {
            s = rv.toInt(&ok);
            if (!ok) return QValidator::Invalid;
        }
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
            updateTime();
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
            updateTime();
            selectSection(static_cast<Section>(sec - 1));
            return true;
        }
    }
}

void TrainTimeEdit::focusInEvent(QFocusEvent* e)
{
    QAbstractSpinBox::focusInEvent(e);
    
    switch (e->reason()) {
    case Qt::BacktabFocusReason:
        selectSection(lastSection());
        break;
    case Qt::TabFocusReason:
    case Qt::OtherFocusReason:   // Activated in QTableView
    case Qt::ActiveWindowFocusReason:
    case Qt::ShortcutFocusReason:
		selectSection(firstSection());
        break;
    }
}

void TrainTimeEdit::focusOutEvent(QFocusEvent* e)
{
    updateTime();
	QAbstractSpinBox::focusOutEvent(e);
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

TrainTimeEdit::Section TrainTimeEdit::positionToSection(int pos) const
{
#if 1

    const QString& txt = lineEdit()->text();
    int sec_idx = 0;
    for (int i = 0; i < txt.length() && i < pos; i++) {
        if (txt.at(i) == ':') {
            sec_idx++;
        }
    }
    assert(sec_idx <= static_cast<int>(lastSection()));
    return static_cast<Section>(sec_idx);
#else
    // 2025.07.24: This is not safe, because we sometimes get non-standard length during input
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
#endif
}

static int integer_digits_dec(int val)
{
    int c = 1;
    while (val /= 10) c++;
    return c;
}

bool TrainTimeEdit::atEndOfNonLastSection(int pos) const
{
    auto sec = positionToSection(pos);
    if (sec == lastSection())
        return false;
    const QString& txt = lineEdit()->text();
    int next_delim = txt.indexOf(':', pos);
    if (next_delim < 0) next_delim = txt.size();
    int last_delim = pos == 0 ? -1 : txt.lastIndexOf(':', pos - 1);

    if (pos < last_delim)
        return false;

    // Now, the position is at the end of the current section

    int curr_sec_length = next_delim - last_delim - 1;

    if (sec == HourSection) {
        // Special processing for hour section: we move to next section if it is impossible to add another character after it
		int max_sec_length = integer_digits_dec(m_maxHours);
        if (curr_sec_length >= max_sec_length) {
            return true;
        }
        else if (curr_sec_length + 1 == max_sec_length) {
            // test whether we can add another digit; the hour section SHOULD starts from zero position
			int hour_val = txt.left(next_delim).toInt();  // this should be safe because we have validated before
            return (hour_val * 10 >= m_maxHours);
        }
        else {
            return false;
        }
    }
    else {
        // For minute and second section, the maximal length is always 2
        return curr_sec_length >= 2;
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
        emit timeChanged(m_time);  // seems: updateTime() will NOT call onTextChanged() slot, and we should emit it here!
    }
}

QSize TrainTimeEdit::sizeHint() const
{
    int h = lineEdit()->sizeHint().height();
    QString s = TrainTime(m_maxHours, 0, 0).toString(m_format) + ' ';
    QFontMetrics fm = lineEdit()->fontMetrics();
    int w = fm.horizontalAdvance(s);
    QSize hint(w + 3, h);
    QStyleOptionSpinBox opt;
    initStyleOption(&opt);
    hint = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this);
    return hint;
}

void TrainTimeEdit::updateTime()
{
    // 2025.08.07: we do not allow invalid time here
    if (m_time.isNull()) {
        m_time = TrainTime::fromSecondsSinceStart(0);
    }
    int currrent_cursor_pos = lineEdit()->cursorPosition();
    const QString old = lineEdit()->text();
    lineEdit()->setText(m_time.toString(m_format));
    if (old != lineEdit()->text())
        lineEdit()->setCursorPosition(qMin(currrent_cursor_pos, lineEdit()->text().length()));
}

void TrainTimeEdit::onTextChanged()
{
    QString t = lineEdit()->text();
    auto pos = lineEdit()->cursorPosition();
    int p = 0;
    auto stat = validate(t, p);
    if (stat == QValidator::Acceptable) {
        // Indicate the input is valid
        auto tm = TrainTime::fromString(t);
        if (tm != m_time) {
            m_time = tm;
			emit timeChanged(m_time);
        }
    }
    else {
        updateTime();   // Update the text if we get invalid characters
    }
    //updateTime();
    // selectCurrentSection();
    lineEdit()->setCursorPosition(pos);

    if (atEndOfNonLastSection(pos)) {
        focusNextChild();
    }
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
	return positionToSection(lineEdit()->cursorPosition());
}
