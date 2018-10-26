#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <regex>
#include <iterator>

#include "LoadMaterial.h"

#include "strtk.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::filesystem;


#pragma once
class LoadTaskList
{
public:
	LoadTaskList();
	~LoadTaskList();

	struct tasklist
	{
		string id; /* Composite id of line counter, '|', and filename */
		string itemNo;
		string areaLabel;			/* is event member */
		string pmrClass;
		string bomGroup;			/* is event member */
		string bomDenomination;
		string type;
		string docNo;
		int worktime;
		int serviceInterval;
		string serviceAction;		/* is event member */
		string serviceDescription;	/* is event member */
		string materialNumber;		/* is event member */
		string materialDescription;
		int materialQuantity;		/* is event member */
		string materialFamily;		/* is event member */
	};

	static const unordered_map<string, tasklist> processTaskLists(
		unordered_map<LONG64, LoadMaterial::material> &NUM_Map)
	{
		// Get task list file names in the directory 'TaskLists'
		vector<string> fileNames = loopDirectory();
		// cout << "Vector 'fileNames' now stores " << (int)fileNames.size() << " task list file names.\n";

		unordered_map<string, tasklist> taskList_map;

		// Process the task lists
		tasklist t;
		
		// Initiate string search and replace
		static string subStringToRemove = ";;";
		static string subStringToReplace = "; ;";

		// **************** BEGIN LOOP OF FILE NAMES ****************
		for (auto& f : fileNames) {
			string last = f.substr(f.length() - 3);
			// Only process .csv files
			if (last == "csv")
			{
				//string text_file_name = ("TaskLists/" + f);
				//string text_file_name = ("C:/Users/SEPALMM/source/repos/CompileMtrlList/x64/Debug/TaskLists/" + f);
				string text_file_name = ("C:/Users/Yoko/source/repos/CompileMaterialList/x64/Debug/TaskLists/" + f);
				cout << "Processing task list " << f << "..." << endl;

				// ETL:
				// Transform: Delete all ';'. Delete header row.
				// Load task list data

				int lineCounter = 1;
				int failedToParseCounter = 0;
				bool isParsingOK = true;
				//strtk::for_each_line(text_file_name, [&](const string& line)
				strtk::for_each_line(text_file_name, [&](string& line) // Cannot be constant as regex will do modifications
				{
					if (isParsingOK)
					{
						// Make composite id of line counter, '|', and filename
						string id = to_string(lineCounter) + '|' + f;
						lineCounter++;

						// Replace with regex any empty fields / cells / tokens in the line
						//cout << "old line content is: " << line << endl;
						regex e("^(;)([^ ]*)");   // matches lines beginning by ";"
						string result;
						regex_replace(back_inserter(result), line.begin(), line.end(), e, " ;$2"); // Inserts space at beginning of line if position is empty
						line = result; // overwrite old line with result

						regex e1("(;)$([^ ]*)");   // matches lines ending by ";"
						string result1;
						regex_replace(back_inserter(result1), line.begin(), line.end(), e1, ";0$2"); // Inserts a 0 at end of line if position is empty (assuming strictly 14 columns in total)
						line = result1; // overwrite old line with result
						
						//Insert space in any empty column to make sure line is parseable
						// Need to repeat twice for odd oocurances
						boost::replace_all(line, subStringToRemove, subStringToReplace);
						boost::replace_all(line, subStringToRemove, subStringToReplace);

						//cout << "NEW line content is:" << line << endl;


						// Do file parsing
						if (strtk::parse(line, ";", t.itemNo, t.areaLabel, t.pmrClass, t.bomGroup, t.bomDenomination,
							t.type, t.docNo, t.worktime, t.serviceInterval, t.serviceAction, t.serviceDescription, t.materialNumber,
							t.materialDescription, t.materialQuantity)) {
							failedToParseCounter = 0; // reset to allow for a new fresh block of lines

							// Prefix service action with 'A','B', or 'C' to sort in natural order
							renameServiceAction(&t.serviceAction);

							// Lookup material number key search
							//cout << t.materialNumber << "\t->\t";
							if (!(t.materialNumber == " ")) {

								// Convert to TP_NUM format
								string convNum = insertZeroesAfterFirstDigit(removeHyphens(removeSpaces(t.materialNumber)));

								/* Search NUM_Map only if materialNumber is a string of (9-11) numbers,
							else there will be an exception when casting a non-digit string to LONG64 */
								if (regex_match(convNum, regex("^\\d{9,11}$")))
								{
									unordered_map<LONG64, LoadMaterial::material>::const_iterator got = NUM_Map.find(stoll(convNum));
									if (got == NUM_Map.end()) {
										cout << "'" << convNum << "' Not found in NUM_Map\t" << endl;
										/* RETURN EXISTING NUMBER FORMAT SURROUNDED BY '?' MARKS */
										t.materialNumber = "<?" + t.materialNumber + "?>";
									}
									else {
										// Return BW formatted Material number, and Material family
										/*cout << got->first << " is BW:\t" << got->second.materialNumberBW
											<< ":\t" << got->second.materialFamily << endl;*/
										t.materialNumber = got->second.materialNumberBW;
										t.materialFamily = got->second.materialFamily;
										if (t.materialDescription == " ")
										{
											t.materialDescription = got->second.materialDescription
												+ " <Description missing in Task List>";
										}
									}
								}
								else
								{
									/* RETURN EXISTING NUMBER FORMAT SURROUNDED BY '?' MARKS */
									t.materialNumber = "<?" + t.materialNumber + "?>";
								}

							}
							else if (t.materialNumber == " ")
							{
								// Initialize to overwrite old memory contents
								t.materialFamily = " ";
							}
							// Print (view) material number after lookup algorithm
							//if (t.materialNumber != " ") { cout << "Using:\t" << t.materialNumber << endl; }

							taskList_map.insert({ id, t });
						}
						else {
							//cerr << "Failed to parse (skipped) line: " << line << endl;
							failedToParseCounter++;
							// Limit runaway parsing loops (max allowed empty rows acc to standard is 10)
							if (failedToParseCounter > 20)
							{
								//cout << "Break out of runaway parsing loop initiated..." << endl;
								isParsingOK = false;
							}
						}
					}
				});
			}
			// **************** END LOOP OF FILE NAMES ****************
		}
		cout << "Created a PMR matrix of " << taskList_map.size() << " rows by 15 columns." << endl;

		/*for (auto& t : taskList_map) {
			cout << t.first << "--> " << t.second.itemNo << ": " << t.second.areaLabel << ": " << t.second.pmrClass << ": " << t.second.bomGroup
				<< ": " << t.second.bomDenomination << ": " << t.second.type << ": " << t.second.docNo << ": " << t.second.worktime << ": " << t.second.serviceInterval << ": " << t.second.serviceAction << ": " << t.second.serviceDescription << ": " << t.second.materialNumber << ": " << t.second.materialDescription << ": " << t.second.materialQuantity
				<< ":...\n";
		}
		cout << endl;*/

		return taskList_map;
	}

private:
	static vector<string> loopDirectory() {

		//path p("TaskLists");
		//path p("C:/Users/SEPALMM/source/repos/CompileMtrlList/x64/Debug/TaskLists");
		path p("C:/Users/yoko/source/repos/CompileMaterialList/x64/Debug/TaskLists");

		vector<string> v;

		try
		{
			if (exists(p))
			{
				if (is_regular_file(p))
					cout << p << " size is " << file_size(p) << '\n';

				else if (is_directory(p))
				{
					cout << p << " is a directory containing:\n";

					for (auto&& x : directory_iterator(p))
						v.push_back(x.path().filename().string());

					sort(v.begin(), v.end());

					for (auto&& x : v)
						cout << "    " << x << '\n';
				}
				else
					cout << p << " exists, but is not a regular file or directory\n";
			}
			else
				cout << p << " directory does not exist, please check.\n";
		}

		catch (const filesystem_error& ex)
		{
			cout << ex.what() << '\n';
		}

		return v;
	}

	/*
	Prefix service action with 'A','B', or 'C' to sort in natural order
	*/
	static void renameServiceAction(string * p) {
		//std::string str = *p;
		boost::algorithm::to_upper(*p);

		if (*p == "CHECK")
		{
			*p = "A_CHECK";
		}
		else if (*p == "TURN")
		{
			*p = "B_TURN";
		}
		else if (*p == "CHANGE")
		{
			*p = "C_CHANGE";
		}
		else
		{
			*p = "Z";
		}
	}

	static string removeSpaces(string str) {
		// remove all spaces from string 'str'
		str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
		return str;
	}

	static string removeHyphens(string str) {
		// remove all hyphens from string 'str'
		str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
		return str;
	}

	static string insertZeroesAfterFirstDigit(string str) {
		// insert zeroes after first character in string such that length of 'str' becomes 11
		string tenZeros = "0000000000";
		// std::cout << str.length() << " is length of number" << std::endl;

		if (str.length() <= 11)
		{
			if (str.length() == 11)
			{
				return str;
			}
			else if (str.length() == 10 && str.substr(0, 1) != "3") /* doesn't start with 3 */
			{
				str.insert(1, tenZeros, 9);
			}
			else if (str.length() == 9 && str.substr(0, 1) != "9") /* doesn't start with 9 */
			{
				str.insert(1, tenZeros, 8);
			}
			else if (str.length() == 8)
			{
				str.insert(1, tenZeros, 7);
			}
			else if (str.length() == 7)
			{
				str.insert(1, tenZeros, 6);
			}
			else if (str.length() == 6)
			{
				str.insert(1, tenZeros, 5);
			}
			else if (str.length() == 5)
			{
				str.insert(1, tenZeros, 4);
			}
			else if (str.length() == 4)
			{
				str.insert(1, tenZeros, 3);
			}
			else if ((str.length() == 10 && str.substr(0, 1) == "3") ||
				(str.length() == 9 && str.substr(0, 1) == "9"))
			{
				return str;
			}
			else
			{
				cerr << "Material number '" << str << "', length is less than 4 digits and out of range, cannot pad zeroes..." << endl;
				return str + " >>>";
			}
		}
		else
		{
			cerr << "Material number '" << str << "', length is over 11 digits and out of range, cannot pad zeroes..." << endl;
			return str + " <<<";
		}
		return str;
	}

};
