/* This class loads material data from a csv file prepared from
application 'NumberParser.exe'. The data is input to a lookup map
where keys are formatted in NUM-format.
The rh side has three number formats, NUM-, BW -, and TP-format,
and in addition also material description, mpg, assortment group,
PG, and material family. */

#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

#include "strtk.hpp"

using namespace std;

#pragma once
class LoadMaterial
{
public:
	LoadMaterial();
	~LoadMaterial();

	struct material
	{
		LONG64 materialNumberNUM;
		string materialNumberBW;
		string materialNumberTP;
		string materialDescription;
		string mpg;
		string assortmentGroup;
		double pg;
		string materialFamily;
	};



	static const unordered_map<LONG64, material> processMaterial(void) {

		material mtrl;
		unordered_map<LONG64, material> NUM_Map;

		// ETL:
		// Extract: see java prgm MaterialMaster
		// Transform: Delete all ';'. Fill any missing descriptions. Delete header row.
		// Load material data
		//string text_file_name = "MaterialList/MaterialMaster.csv";
		string text_file_name = "C:/Users/Yoko/source/repos/CompileMaterialList/x64/Debug/MaterialList/MaterialMaster.csv";

		cout << "Loading material data..." << endl;

		strtk::for_each_line(
			text_file_name,
			[&](const string& line)
		{
			if (strtk::parse(line, ";", mtrl.materialNumberNUM, mtrl.materialNumberBW, mtrl.materialNumberTP, mtrl.materialDescription, mtrl.mpg, mtrl.assortmentGroup, mtrl.pg, mtrl.materialFamily))
				// Make a K,V map with NUM formatted keys
				NUM_Map.insert({ mtrl.materialNumberNUM, mtrl });
			else
				cerr << "Failed to parse (skipped): " << line << endl;
		});

		cout << "Succesfully loaded material data in a matrix of size " << NUM_Map.size() << " rows by 8 columns." << endl;

		/*for (auto& m : NUM_Map) {
			cout << m.first << ": " << m.second.materialNumberNUM << ": " << m.second.materialNumberBW << ": " << m.second.materialNumberTP << ": " << m.second.materialDescription << ": " << m.second.mpg << ": " << m.second.assortmentGroup << ": " << m.second.pg << ": " << m.second.materialFamily << "\n";
		}
		cout << endl;*/

		return NUM_Map;
	}

};
