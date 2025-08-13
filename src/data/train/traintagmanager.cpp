#include "traintagmanager.h"

#include <QJsonArray>
#include <QDebug>

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
			m_tags[tag.name()] = std::make_shared<TrainTag>(tag);
		}
	}
}

std::shared_ptr<TrainTag> TrainTagManager::find(const QString& name) const
{
	auto it = m_tags.find(name);
	if (it != m_tags.end()) {
		return it->second;
	}
	return nullptr;
}

std::shared_ptr<TrainTag> TrainTagManager::findOrCreate(const QString& name)
{
	auto it = m_tags.find(name);
	if (it != m_tags.end()) {
		return it->second;
	}
	// Not found, create a new tag
	const auto& res = m_tags.emplace(name, std::make_unique<TrainTag>(name));
	return res.first->second;
}

bool TrainTagManager::contains(const QString& name)
{
	return m_tags.contains(name);
}

void TrainTagManager::removeTag(const QString& name)
{
	if (auto itr= m_tags.find(name); itr != m_tags.end()) {
		m_tags.erase(itr);
	}
	else {
		qWarning() << "TrainTagManager::removeTag: tag not found: " << name;
	}
}

void TrainTagManager::addTag(std::shared_ptr<TrainTag> tag)
{
	m_tags[tag->name()] = tag;
}
