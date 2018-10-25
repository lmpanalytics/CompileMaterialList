#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <locale.h>

#include "ServiceEvent.h"

#include <boost/algorithm/string.hpp>

using namespace std;

#pragma once
class Logic
{
public:
	Logic();
	~Logic();

	enum class Justification {
		CHECK_action_of_Function,
		CHECK_action_with_or_without_subsequent_CHANGE_TURN_action,
		CHECK_action_with_or_without_subsequent_CHANGE_TURN_action_on_3_Piston_Machines,
		CHECK_action_with_or_without_subsequent_CHANGE_TURN_action_on_5_Piston_Machines,
		CHANGE_action_with_service_description_containing_a_KIT,
		No_matching_SKU_rule_found
	};

	struct StockKeepingUnit
	{
		string materialNumber;
		string materialDescription;
		int materialQuantity;
		string materialFamily;
		Justification justification;
	};

	static const int processEvents(
		map<string, ServiceEvent::serviceEvent> &eventMap)
	{
		bool is_SKU = false;
		bool is_SKU_Kit = false;
		bool is_function_check = false;
		string beginningBoM = "";
		string beginningLabel = "";

		StockKeepingUnit sku;

		/* K: materialNumber, V: sku */
		map<string, StockKeepingUnit> skuMap;
		map<string, StockKeepingUnit> nonSkuMap;

		// loop through the event map and assign StockKeepingUnit struct to skuMap
		for (auto& e : eventMap) {

			string * pServiceAction = &e.second.serviceAction;
			string * pServiceDescription = &e.second.serviceDescription;
			string * pMaterialNumber = &e.second.materialNumber;
			string * pMaterialDescription = &e.second.materialDescription;
			string * pBomGroup = &e.second.bomGroup;
			string * pAreaLabel = &e.second.areaLabel;
			int * pMaterialQuantity = &e.second.materialQuantity;
			string * pMaterialFamily = &e.second.materialFamily;

			boost::algorithm::to_upper(*pServiceDescription);
			boost::algorithm::to_upper(*pMaterialDescription);

			/* ******** Handle CHECK and FUNCTION (assign all of its BoM items SKU status) ******** */
			if (is_function_check || (*pServiceAction == "A_CHECK" &&
				(*pServiceDescription).find("FUNCTION") != string::npos))
			{
				// Get the beginning BomGroup and areaLabel
				if (*pServiceAction == "A_CHECK" &&
					(*pServiceDescription).find("FUNCTION") != string::npos)
				{
					beginningBoM.assign(*pBomGroup);
					beginningLabel.assign(*pAreaLabel);
					//cout << "ASSIGNING BOM: " << beginningBoM << ", ASSIGNING AREA LABEL: " << *pAreaLabel << "\n";		
				}

				// Activate function check 
				if (beginningBoM == *pBomGroup && beginningLabel == *pAreaLabel)
				{
					is_function_check = true;

					// Add to map if there is a material number larger than 3 spaces and/or characters
					if ((*pMaterialNumber).length() > 3) {
						//cout << e.first << "\n";
						sku.materialNumber = *pMaterialNumber;
						sku.materialDescription = *pMaterialDescription;
						sku.materialQuantity = *pMaterialQuantity;
						sku.materialFamily = *pMaterialFamily;
						sku.justification = Justification::CHECK_action_of_Function;
						map<string, StockKeepingUnit>::iterator got = skuMap.find(*pMaterialNumber);
						if (got != skuMap.end()) {
							/* map contains key, so get existing quantity,
							add the new quantity, and re-insert sku */
							int storedQty = got->second.materialQuantity;
							int newQty = storedQty + *pMaterialQuantity;
							got->second.materialQuantity = newQty;
						}
						else {
							// map doesn't contain key, so insert sku as is
							skuMap.insert({ *pMaterialNumber, sku });
						}
					}
				}
				else if (beginningBoM != *pBomGroup || beginningLabel != *pAreaLabel)
				{
					is_function_check = false;
					//cout << "********* BREAKING FUNCTION CHECK LOOP *********\n" << endl;
				}
			}

			/* ******** Handle Singleton CHECK action (or followed by TURN and/or CHANGE) equals an SKU ******** */
			else if (is_SKU || *pServiceAction == "A_CHECK")
			{
				// Get the beginning BomGroup and areaLabel
				if (*pServiceAction == "A_CHECK")
				{
					beginningBoM.assign(*pBomGroup);
					beginningLabel.assign(*pAreaLabel);
					//cout << "ASSIGNING BOM: " << beginningBoM << ", ASSIGNING AREA LABEL: " << *pAreaLabel << "\n";		
				}

				// Activate SKU check 
				if (beginningBoM == *pBomGroup && beginningLabel == *pAreaLabel)
				{
					is_SKU = true;

					// Add to map if there is a material number larger than 3 spaces and/or characters
					if ((*pMaterialNumber).length() > 3) {
						//cout << e.first << "\n";
						sku.materialNumber = *pMaterialNumber;
						sku.materialDescription = *pMaterialDescription;
						sku.materialQuantity = *pMaterialQuantity;
						sku.materialFamily = *pMaterialFamily;
						sku.justification = Justification::CHECK_action_with_or_without_subsequent_CHANGE_TURN_action;
						map<string, StockKeepingUnit>::iterator got = skuMap.find(*pMaterialNumber);
						if (got != skuMap.end()) {
							/* map contains key, so get existing quantity,
							add the new quantity, and re-insert sku */
							int storedQty = got->second.materialQuantity;
							int newQty = storedQty + *pMaterialQuantity;
							got->second.materialQuantity = newQty;
						}
						else {
							// map doesn't contain key, so insert sku as is
							skuMap.insert({ *pMaterialNumber, sku });
						}
					}
				}
				else if (beginningBoM != *pBomGroup || beginningLabel != *pAreaLabel)
				{
					is_SKU = false;
					//cout << "********* BREAKING CHECK LOOP *********\n" << endl;
				}
			}

			/* ******** Put item to SKUmap if service description contains 'kit' ******** */
			else if (is_SKU_Kit
				|| (*pServiceDescription).find("KIT") != string::npos
				|| (*pMaterialDescription).find("KIT") != string::npos)
			{
				// Get the beginning BomGroup and areaLabel
				if ((*pServiceDescription).find("KIT") != string::npos)
				{
					beginningBoM.assign(*pBomGroup);
					beginningLabel.assign(*pAreaLabel);
					//cout << "ASSIGNING BOM: " << beginningBoM << ", ASSIGNING AREA LABEL: " << *pAreaLabel << "\n";
				}

				// Activate SKU check 
				if (beginningBoM == *pBomGroup && beginningLabel == *pAreaLabel)
				{
					is_SKU_Kit = true;

					// Add to map if there is a material number larger than 3 spaces and/or characters
					if ((*pMaterialNumber).length() > 3) {
						//cout << e.first << "\n";
						sku.materialNumber = *pMaterialNumber;
						sku.materialDescription = *pMaterialDescription;
						sku.materialQuantity = *pMaterialQuantity;
						sku.materialFamily = *pMaterialFamily;
						sku.justification = Justification::CHANGE_action_with_service_description_containing_a_KIT;
						map<string, StockKeepingUnit>::iterator got = skuMap.find(*pMaterialNumber);
						if (got != skuMap.end()) {
							/* map contains key, so get existing quantity,
							add the new quantity, and re-insert sku */
							int storedQty = got->second.materialQuantity;
							int newQty = storedQty + *pMaterialQuantity;
							got->second.materialQuantity = newQty;
						}
						else {
							// map doesn't contain key, so insert sku as is
							skuMap.insert({ *pMaterialNumber, sku });
						}
					}
				}
				else if (beginningBoM != *pBomGroup || beginningLabel != *pAreaLabel)
				{
					is_SKU_Kit = false;
					//cout << "********* BREAKING KIT LOOP *********\n" << endl;
				}
			}

			/* *************** Handle NON-SKU ITEMS *************** */
			else if (!is_function_check && !is_SKU && !is_SKU_Kit)
			{
				//cout << "Material: '" << *pMaterialNumber << " " << *pMaterialDescription << "'\tis no SKU\n";

				// Add to map if there is a material number larger than 3 spaces and/or characters
				if ((*pMaterialNumber).length() > 3) {
					//cout << e.first << "\n";
					sku.materialNumber = *pMaterialNumber;
					sku.materialDescription = *pMaterialDescription;
					sku.materialQuantity = 0;
					sku.materialFamily = *pMaterialFamily;
					sku.justification = Justification::No_matching_SKU_rule_found;
					map<string, StockKeepingUnit>::iterator got = nonSkuMap.find(*pMaterialNumber);
					if (got != nonSkuMap.end()) {
						// map contains key, do nothing...
					}
					else {
						// map doesn't contain key, so insert sku as is
						nonSkuMap.insert({ *pMaterialNumber, sku });
					}
				}
			}

		}
		/* ***************** APPLY QUANTIFICATION LOGIC ***************** */
		for (auto& m : skuMap) {
			string * pMaterialNumber = &m.second.materialNumber;
			int * pMaterialQuantity = &m.second.materialQuantity;
			string * pMaterialFamily = &m.second.materialFamily;
			Justification * pJustification = &m.second.justification;

			boost::algorithm::to_upper(*pMaterialFamily);

			if (*pMaterialFamily == "PISTON"
				|| *pMaterialFamily == "PISTON SEAL"
				|| *pMaterialFamily == "COMPRESSION RING"
				|| *pMaterialFamily == "SUPPORT RING"
				|| *pMaterialFamily == "GUIDE BAND"
				|| *pMaterialFamily == "V-BELT"
				|| *pMaterialFamily == "BELLOW")
			{
				// update map qty according to quantification logic
				map<string, StockKeepingUnit>::iterator got = skuMap.find(*pMaterialNumber);
				if (got != skuMap.end()) {
					/* map contains key, so get existing quantity,
					re-calculate the new quantity, and update. */
					int storedQty = got->second.materialQuantity;

					if (storedQty % 3 == 0)
					{
						// 3-Piston Homogenizer
						got->second.materialQuantity = 3;
						got->second.justification = Justification::CHECK_action_with_or_without_subsequent_CHANGE_TURN_action_on_3_Piston_Machines;
					}
					else
					{
						// 5-Piston Homogenizer
						got->second.materialQuantity = 5;
						got->second.justification = Justification::CHECK_action_with_or_without_subsequent_CHANGE_TURN_action_on_5_Piston_Machines;
					}
				}

			}
			else
			{
				/* Calculate stock quantity according to default formula
				and update map materialQuantity accordingly */
				map<string, StockKeepingUnit>::iterator got = skuMap.find(*pMaterialNumber);
				if (got != skuMap.end()) {
					/* map contains key, so get existing quantity,
					re-calculate the new quantity, and update. */
					int storedQty = got->second.materialQuantity;
					int newQty = ceil((double)storedQty / 20);
					got->second.materialQuantity = newQty;
				}
			}

		}

		/* ***************** PREPARE FILE TIMESTAMP ***************** */
		std::time_t t = std::time(nullptr);
		char mbstr[100];

		/* ***************** WRITE SKU MAPS TO FILE ***************** */
			// Write file
		if (std::strftime(mbstr, sizeof(mbstr), "%F_%H%M%S", std::localtime(&t))) {
			/* ***************** WRITE SKU MAP TO FILE ***************** */
			cout << "Write to file..." << endl;
			string timeStamp(mbstr);
			strtk::util::timer timerExport;
			timerExport.start();
			std::ofstream myfile;
			string fileName = "RecommendedPartsTable_" + timeStamp + ".csv";
			myfile.open(fileName);

			// header
			myfile << "MTRL NUMBER BW" << ";" << "MTRL DESCRIPTION" << ";" << "QUANTITY" << ";" << "JUSTIFICATION" << "\n";

			// content
			int fileLineCounter = 0;
			for (auto& m : skuMap) {
				//std::cout << m.first << "\n";
				string str = "";
				switch (m.second.justification)
				{
				case Justification::CHECK_action_of_Function:
					str = "CHECK action of Function";
					break;
				case Justification::CHECK_action_with_or_without_subsequent_CHANGE_TURN_action:
					str = "CHECK action with or without subsequent CHANGE or TURN action";
					break;
				case Justification::CHECK_action_with_or_without_subsequent_CHANGE_TURN_action_on_3_Piston_Machines:
					str = "CHECK action with or without subsequent CHANGE or TURN action on 3 Piston Machines";
					break;
				case Justification::CHECK_action_with_or_without_subsequent_CHANGE_TURN_action_on_5_Piston_Machines:
					str = "CHECK action with or without subsequent CHANGE or TURN action on 5 Piston Machines";
					break;
				case Justification::CHANGE_action_with_service_description_containing_a_KIT:
					str = "CHANGE action with service description containing a KIT";
					break;
				case Justification::No_matching_SKU_rule_found:
					str = "No matching SKU rule found";
					break;
				default:
					str = "No matching SKU rule found";
					break;
				}

				myfile << m.second.materialNumber << ";" << m.second.materialDescription << ";"
					<< m.second.materialQuantity << ";" << str << "\n";

				fileLineCounter++;
			}

			// footer
			myfile << "\nThis is the raw list of recommended SKUs based on materials " <<
				"included in imported tasklists, \nto be discussed further with " <<
				"a Parts Control Specialist and the Customer." << "\n";

			timerExport.stop();
			printf("[write file 'RecommendedPartsTable.csv' ] Wrote %d lines in Time %8.5fsec\n", fileLineCounter, timerExport.time());


			/* ***************** WRITE NON SKU MAP TO FILE ***************** */
				// Write file
			cout << "Write to file..." << endl;
			strtk::util::timer timerExport1;
			timerExport1.start();
			std::ofstream myfile1;
			string fileName1 = "NonSKUTable_" + timeStamp + ".csv";
			myfile1.open(fileName1);

			// header
			myfile1 << "MTRL NUMBER BW" << ";" << "MTRL DESCRIPTION" << ";" << "JUSTIFICATION" << "\n";

			// content
			int fileLineCounter1 = 0;
			for (auto& m : nonSkuMap) {
				//std::cout << m.first << "\n";
				string str = "Not applicable as SKU, based on Task List rule logic";
				myfile1 << m.second.materialNumber << ";" << m.second.materialDescription << ";" << str << "\n";
				fileLineCounter1++;
			}

			// footer
			myfile1 << "\nThis is the raw list of NON-recommended SKUs based on materials " <<
				"included in imported tasklists, \nto be discussed further with " <<
				"a Parts Control Specialist and the Customer." << "\n";

			timerExport1.stop();
			printf("[write file 'NonSKUTable.csv' ] Wrote %d lines in Time %8.5fsec\n", fileLineCounter1, timerExport1.time());
		}
		return 0;
	}
};
