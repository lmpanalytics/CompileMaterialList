// CompileMaterialList.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "LoadMaterial.h"
#include "LoadTaskList.h"
#include "ServiceEvent.h"
#include "Logic.h"

#include <iostream>

using namespace std;

int main(int argc, char*argv[])
{
	unordered_map<LONG64, LoadMaterial::material> NUM_Map = LoadMaterial::processMaterial();
	unordered_map<string, LoadTaskList::tasklist> taskListMap = LoadTaskList::processTaskLists(NUM_Map);
	map<string, ServiceEvent::serviceEvent> eventMap = ServiceEvent::makeSortedEventMap(taskListMap);
	Logic::processEvents(eventMap); /* Prints list of SKUs */

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
