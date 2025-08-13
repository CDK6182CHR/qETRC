#include <memory>
#include <map>

#include "traintag.h"

/**
 * 2025.08.13  Experimental: manager for train tags. Similar to TypeManager.
 * This class is mainly a container for TrainTag objects. The train tag objects are stored in a map with their names as keys.
 * Any train tag that is assigned to one train should be stored in this manager.
 * For safety we use shared_ptr instead of unique_ptr, like TrainType. Because we cannot ensure the TrainTag that 
 * removed by the user does not assigned to any train.
 */
class TrainTagManager 
{
	using TagMap = std::map<QString, std::shared_ptr<TrainTag>>;
	TagMap m_tags;

public:
	TrainTagManager() = default;

	QJsonObject toJson() const;
	void fromJson(const QJsonObject& obj);

	/**
	 * Find a tag by name; nullptr if not found.
	 */
	std::shared_ptr<TrainTag> find(const QString& name)const;

	/**
	 * Find a tag by name, or create a new tag if it does not exist.
	 */
	std::shared_ptr<TrainTag> findOrCreate(const QString& name);
};