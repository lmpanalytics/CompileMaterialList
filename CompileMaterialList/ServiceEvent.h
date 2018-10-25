#include <iostream>
#include <map>
#include <unordered_map>

#include "LoadTaskList.h"

using namespace std;

#pragma once
class ServiceEvent
{
public:
	ServiceEvent();
	~ServiceEvent();

	struct serviceEvent
	{
		string areaLabel;
		string bomGroup;
		string serviceAction;
		string serviceDescription;
		string materialNumber;
		string materialDescription;
		int materialQuantity;
		string materialFamily;
	};

	static const map<string, serviceEvent> makeSortedEventMap(
		unordered_map<string, LoadTaskList::tasklist> &taskListMap)
	{
		serviceEvent sEvent;
		map<string, serviceEvent> eventMap;

		// loop through the tasklist map and assign serviceEvent struct to eventMap
		for (auto& t : taskListMap) {
			// make compound id to sort against
			string id = t.second.bomGroup + "|" + t.second.areaLabel
				+ "|" + t.second.materialFamily + "|" + t.second.serviceAction
				+ "|" + t.second.materialNumber + "|" + t.first;

			sEvent.areaLabel = t.second.areaLabel;
			sEvent.bomGroup = t.second.bomGroup;
			sEvent.serviceAction = t.second.serviceAction;
			sEvent.serviceDescription = t.second.serviceDescription;
			sEvent.materialNumber = t.second.materialNumber;
			sEvent.materialDescription = t.second.materialDescription;
			sEvent.materialQuantity = t.second.materialQuantity;
			sEvent.materialFamily = t.second.materialFamily;

			eventMap.insert({ id, sEvent });
		}

		return eventMap;
	}
};
