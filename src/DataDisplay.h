/*
 * DataDisplay.h
 *
 *  Created on: Mar. 15, 2024
 *      Author: Alexandre
 */

#ifndef SRC_DATADISPLAY_H_
#define SRC_DATADISPLAY_H_

#define rows 25
#define columns 25

#include "Aircraft.h"
#include <string>
#include <vector>

using namespace std;

class DataDisplay {
public:
	DataDisplay();
	int code;
	static void* MapInitializer(void* args);
	static void* dataDisplayInitializer(void* args);
	void* listenForAircraftMap();
	void* listen();
	void initMap(string (&airspace)[rows][columns]);
	void clearPrevious(string (&airspace)[rows][columns], int indexI, int indexJ, string flightId);
	string updateMap(vector<Aircraft*>& planes, string (&airspace)[rows][columns]);
	void writeMap(string mapAsString);

	virtual ~DataDisplay();
};

#endif /* SRC_DATADISPLAY_H_ */
