/*
 * DataDisplay.cpp
 *
 *  Created on: Mar. 15, 2024
 *      Author: Alexandre
 */

#include "DataDisplay.h"
#include "Aircraft.h"

#include <string>
#include <vector>
#include <iostream>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default"

DataDisplay::DataDisplay() {

}

void* DataDisplay::dataDisplayInitializer(void* args) {
	DataDisplay* dataDisplay = (DataDisplay*)args;
	dataDisplay->listen();
	return NULL;
}

void* DataDisplay::MapInitializer(void* args) {
	DataDisplay* dataDisplay = (DataDisplay*)args;
	dataDisplay->listenForAircraftMap();
	return NULL;
}

/**
 * Function used to display the Map of Aircrafts
 */
void* DataDisplay::listenForAircraftMap() {
	string airspace[rows][columns];
	initMap(airspace);

	string attachPoint = string(ATTACH_POINT) + "_MAP";

	name_attach_t *attach;

	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		perror("DataDisplay (listenForAircraftMap): Error occurred while creating the attach point");
	}
	vector<AircraftData> aircrafts;
	AircraftData aircraftCommand;
	aircrafts.push_back(aircraftCommand);
	aircrafts[0].header.type = 0x05;
	aircrafts[0].header.subtype = 0x05;

	while (true) {

		int rcvid = MsgReceive(attach->chid, &aircrafts, sizeof(aircrafts), NULL);

		if (rcvid == -1) { // Error condition, exit
			break;
		}

		if (rcvid == 0) {/* Pulse received */
			switch (aircraftCommand.header.code) {
			case _PULSE_CODE_DISCONNECT:
				/*
				 * A client disconnected all its connections (called
				 * name_close() for each name_open() of our name) or
				 * terminated
				 */
				ConnectDetach(aircraftCommand.header.scoid);
				continue;
			case _PULSE_CODE_UNBLOCK:
				/*
				 * REPLY blocked client wants to unblock (was hit by
				 * a signal or timed out).  It's up to you if you
				 * reply now or later.
				 */
				break;
			default:
				/*
				 * A pulse sent by one of your processes or a
				 * _PULSE_CODE_COIDDEATH or _PULSE_CODE_THREADDEATH
				 * from the kernel?
				 */
				break;
			}
			continue;
		}

		/* name_open() sends a connect message, must EOK this */
		if (aircraftCommand.header.type == _IO_CONNECT ) {
			MsgReply( rcvid, EOK, NULL, 0 );
			continue;
		}

		/* Some other QNX IO message was received; reject it */
		if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX ) {
			MsgError( rcvid, ENOSYS );
			continue;
		}

		if (aircrafts[0].header.type == 0x05) {
			if (aircrafts[0].header.subtype == 0x05) {

				vector<Aircraft*> receivedAircrafts;

				for (AircraftData aircraftCommand : aircrafts) {
					Aircraft* aircraft = new Aircraft();
					aircraft->setFlightId(aircraftCommand.flightId);
					aircraft->setPositionX(aircraftCommand.positionX);
					aircraft->setPositionY(aircraftCommand.positionY);
					aircraft->setPositionZ(aircraftCommand.positionZ);
					aircraft->setSpeedX(aircraftCommand.speedX);
					aircraft->setSpeedY(aircraftCommand.speedY);
					aircraft->setSpeedZ(aircraftCommand.speedZ);

					receivedAircrafts.push_back(aircraft);

				}

				updateMap(receivedAircrafts, airspace);

				// reply to CommandSystem
				MsgReply(rcvid, EOK, NULL, 0);

				receivedAircrafts.clear();

				// continue to listen for subsequent requests
				continue;
			} else {
				MsgError(rcvid, ENOSYS);
				continue;
			}
		}
	}

	return EXIT_SUCCESS;
}

/**
 * Function used to listen to display a specific aircraft's info (request by Operator)
 */
void* DataDisplay::listen(){

	string attachPoint = string(ATTACH_POINT) + "_datadisplay_";

	name_attach_t *attach;

	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		perror("DataDisplay (listen): Error occurred while creating the attach point");
	}

	AircraftData aircraftCommand;

	while (true) {

		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

		if (rcvid == -1) { // Error condition, exit
			break;
		}

		if (rcvid == 0) {/* Pulse received */
			switch (aircraftCommand.header.code) {
			case _PULSE_CODE_DISCONNECT:
				/*
				 * A client disconnected all its connections (called
				 * name_close() for each name_open() of our name) or
				 * terminated
				 */
				ConnectDetach(aircraftCommand.header.scoid);
				continue;
			case _PULSE_CODE_UNBLOCK:
				/*
				 * REPLY blocked client wants to unblock (was hit by
				 * a signal or timed out).  It's up to you if you
				 * reply now or later.
				 */
				break;
			default:
				/*
				 * A pulse sent by one of your processes or a
				 * _PULSE_CODE_COIDDEATH or _PULSE_CODE_THREADDEATH
				 * from the kernel?
				 */
				break;
			}
			continue;
		}

		/* name_open() sends a connect message, must EOK this */
		if (aircraftCommand.header.type == _IO_CONNECT ) {
			MsgReply( rcvid, EOK, NULL, 0 );
			continue;
		}

		/* Some other QNX IO message was received; reject it */
		if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX ) {
			MsgError( rcvid, ENOSYS );
			continue;
		}

		if (aircraftCommand.header.type == 0x05) {
			if (aircraftCommand.header.subtype == 0x01) {
				// Operator Request: Display an Aircraft's Info (0x05 and 0x01)

				cout << endl << endl << "-----------------------------------------------------------------------------" << endl << endl
						<< "(Operator Request) Received aircraft data: flightId = " << aircraftCommand.flightId
						<< ", PositionX = " << aircraftCommand.positionX
						<< ", PositionY = " << aircraftCommand.positionY
						<< ", PositionZ = " << aircraftCommand.positionZ
						<< ", SpeedX = " << aircraftCommand.speedX
						<< ", SpeedY = " << aircraftCommand.speedY
						<< ", SpeedZ = " << aircraftCommand.speedZ
						<< endl << "-----------------------------------------------------------------------------" << endl << endl;

				// reply to CommandSystem
				MsgReply(rcvid, EOK, NULL, 0);

				// continue to listen for subsequent requests
				continue;
			} else {
				MsgError(rcvid, ENOSYS);
				continue;
			}
		}
	}

	return EXIT_SUCCESS;
}

/**
 * -------------------------------------------------------------------------------------------------
 * MAP FUNCTIONS
 * -------------------------------------------------------------------------------------------------
 */

/**
 * Initialize the map
 */
void DataDisplay::initMap(string (&airspace)[rows][columns]){
	for(int i = 0; i<rows; i++){
		for (int j = 0; j<columns; j++){
			airspace[i][j] = " [  ] ";
		}
	}
}

/**
 * Clear previous
 */
void DataDisplay::clearPrevious(string (&airspace)[rows][columns], int indexI, int indexJ, string flightId){
	for(int i = 0; i<rows; i++){
		for (int j = 0; j<columns; j++){
			if(airspace[i][j] == flightId){
				airspace[i][j] = " [  ] ";
			}
			else{
				// don't do anything
			}
		}

	}
}

/**
 * Write map
 */
void DataDisplay::writeMap(string mapAsString){
	code = creat("/data/home/qnxuser/MapLog.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(code==-1){
		cout<<"Map could not be logged, error"<<endl;
	}else{
		//The below is done to create a buffer to write to a text file according to QNX libraries
		char *blockBuffer = new char[mapAsString.length()+1];
		sprintf(blockBuffer, "%s", mapAsString.c_str());
		write(code, blockBuffer, mapAsString.length()+1);
		write(code, "====", 4);
		write(code, "\n", 1);
		delete[] blockBuffer;
		close(code);
	}
}

/**
 * Update map based on current aircraft positions
 */
string DataDisplay::updateMap(vector<Aircraft*>& planes, string (&airspace)[rows][columns]){

	cout << endl << endl;
	//each cell is 1000 ft
	for(int i = 0; i<rows; i++){
		for (int j = 0; j<columns; j++){
			for(size_t k = 0; k<planes.size(); k++){
				int xCurrent = planes[k]->getPositionX();
				int yCurrent = planes[k]->getPositionY();
				//Fitting them in the 1000 sq. ft cells
				xCurrent /=1000;
				yCurrent /=1000;
				try{
					if(i==yCurrent && j==xCurrent && airspace[i][j] == " [  ] "){
						clearPrevious(airspace, i, j, " [ "+ to_string(planes[k]->getFlightId()) + " ] ");
						airspace[i][j] = " [ "+ to_string(planes[k]->getFlightId()) + " ] ";
					}
					else{
						// don't do anything
					}
				} catch(...){
					//most likely error out of bounds exception, this trycatch block will tell the program to move on
				}
			}
		}
	}

	string mapAsString=""; //This can be made a class variable for later use with timers
	for(int i = 0; i<rows; i++){
		for (int j = 0; j<columns; j++){
			cout << airspace[i][j];
			mapAsString += airspace[i][j]; // concatenating mapAsString with values from the matrix... QNX only wants pure strings to be written in text files, not matrices...
		}
		cout << endl;
		mapAsString += "\n";
	}
	writeMap(mapAsString);
	return mapAsString;

}

DataDisplay::~DataDisplay() {
}

