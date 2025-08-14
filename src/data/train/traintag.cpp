#include "traintag.h"

TrainTag::TrainTag(const QJsonObject& obj)
{
	fromJson(obj);
}

void TrainTag::fromJson(const QJsonObject& obj)
{
	if (obj.contains("name")) {
		m_name = obj.value("name").toString();
	}
	if (obj.contains("note")) {
		m_note = obj.value("note").toString();
	}
}

QJsonObject TrainTag::toJson() const
{
	return QJsonObject {
		{"name", m_name},
		{"note", m_note}
	};
}

bool TrainTag::operator<(const TrainTag& rhs) const
{
	return std::tie(m_name, m_note) < std::tie(rhs.m_name, rhs.m_note);
}

void TrainTag::swapWith(TrainTag& other)
{
	std::swap(m_name, other.m_name);
	std::swap(m_note, other.m_note);
}
