#include "traintagmanager.h"

#include <QJsonArray>

QJsonObject TrainTagManager::toJson() const
{
	QJsonArray tagsArray;
	for (const auto& tagPair : m_tags) {
		tagsArray.append(tagPair.second->toJson());
	}
	return QJsonObject{
		{"tags", tagsArray}
	};
}

void TrainTagManager::fromJson(const QJsonObject& obj)
{
	m_tags.clear();
	if (!obj.contains("tags")) {
		return; // No tags found
	}
	
	const QJsonArray& tagsArray = obj.value("tags").toArray();
	for (const auto& tagValue : tagsArray) {
		if (tagValue.isObject()) {
			TrainTag tag;
			tag.fromJson(tagValue.toObject());
			m_tags[tag.name()] = std::make_unique<TrainTag>(tag);
		}
	}
}

TrainTag* TrainTagManager::find(const QString& name) const
{
	auto it = m_tags.find(name);
	if (it != m_tags.end()) {
		return it->second.get();
	}
	return nullptr;
}

TrainTag* TrainTagManager::findOrCreate(const QString& name)
{
	auto it = m_tags.find(name);
	if (it != m_tags.end()) {
		return it->second.get();
	}
	// Not found, create a new tag
	const auto& res = m_tags.emplace(name, std::make_unique<TrainTag>(name));
	return res.first->second.get();
}
