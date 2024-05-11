/*
 * Radar.h
 *
 *  Created on: Feb. 11, 2024
 *      Author: Alexandre
 */

#ifndef SRC_RADAR_H_
#define SRC_RADAR_H_

#include "Aircraft.h"

#include <vector>

using namespace std;

class Radar {
public:
	Radar();
	Radar(vector<Aircraft> &aircrafts);
	Radar(Aircraft &aircraft);
	static void* radarInitializer(void* args);

	void* pingAircraft();
	AircraftData operatorRequestPingAircraft(int flightId);

	virtual ~Radar();

private:
	Aircraft aircraft;
	vector<Aircraft> aircrafts;
};

#endif /* SRC_RADAR_H_ */
