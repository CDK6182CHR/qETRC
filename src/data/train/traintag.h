#pragma once

#include <compare>

#include <QString>
#include <QJsonObject>


/**
 * 2025.08.13  Experimental  Train tag.
 * A tag is mainly a label for a train, containing any valid string. An optional note may also be added.
 * Different train tags are specified by different names.
 * Currently, we have only name and note attributes.
 */
class TrainTag
{
	QString m_name;
	QString m_note;

public:
	TrainTag() = default;
	TrainTag(const QString& name, const QString& note = {}) : m_name(name), m_note(note) {}
	TrainTag(const TrainTag&) = default;
	TrainTag(TrainTag&&) noexcept = default;

	TrainTag(const QJsonObject& obj);

	void fromJson(const QJsonObject& obj);
	QJsonObject toJson()const;

	TrainTag& operator=(const TrainTag&) = default;
	TrainTag& operator=(TrainTag&&) noexcept = default;
	const QString& name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }
	const QString& note() const { return m_note; }
	void setNote(const QString& note) { m_note = note; }
	bool operator==(const TrainTag& rhs) const {
		return m_name == rhs.m_name && m_note == rhs.m_note;
	}

	bool operator<(const TrainTag& rhs)const;

	void swapWith(TrainTag& other);
};
