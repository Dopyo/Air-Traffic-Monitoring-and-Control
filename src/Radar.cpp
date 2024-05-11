/*
 * Radar.cpp
 *
 *  Created on: Feb. 11, 2024
 *      Author: Alexandre
 */

#include "Radar.h"
#include "Aircraft.h"

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default"

Radar::Radar() {

}

Radar::Radar(vector<Aircraft> &aircrafts) {
	this->aircrafts = aircrafts;
}

Radar::Radar(Aircraft &aircraft) {
	this->aircraft = aircraft;
}

void* Radar::radarInitializer(void* args) {
	Radar* radar = (Radar*)args;
	radar->pingAircraft();
	return NULL;
}

void* Radar::pingAircraft() {

	string attachPoint = string(ATTACH_POINT) + "_RADAR";

	name_attach_t *attach;

	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		perror("Radar (pingAircraft): Error occurred while creating the attach point");
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

		if (aircraftCommand.header.type == 0x08) {
			if (aircraftCommand.header.subtype == 0x08) {

				// Generate the unique attach point name based on flightId
				string attachPoint = string(ATTACH_POINT) + "_" + to_string(aircraftCommand.flightId);

				// 1. establish connection (communication)
				int server_coid;
				if ((server_coid = name_open(attachPoint.c_str(), 0)) == -1) {
					perror("Error occurred while attaching the channel");
				}

				// 2. Set Header Type and Subtype, i.e 0x00 and 0x01 to request aircraft data (speed, position)
				// Talk to Aircraft listen()
				AircraftData aircraftCommand;
				aircraftCommand.header.type = 0x00;
				aircraftCommand.header.subtype = 0x01;

				// 3. Send data expecting position/velocity response
				if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
					cout << "Error while sending the message for aircraft "  << ": " << strerror(errno) << endl;
					name_close(server_coid);
				}


				// Reply to ComputerSystem with Aircraft Information
				MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));

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

AircraftData Radar::operatorRequestPingAircraft(int flightId) {

	// Generate the unique attach point name based on flightId
	string attachPoint = string(ATTACH_POINT) + "_" + to_string(flightId);

	// 1. establish connection (communication)
	int server_coid;
	if ((server_coid = name_open(attachPoint.c_str(), 0)) == -1) {
		perror("Error occurred while attaching the channel");
	}

	// 2. Set Header Type and Subtype, i.e 0x00 and 0x01 to request aircraft data (speed, position)
	AircraftData aircraftCommand;
	aircraftCommand.header.type = 0x00;
	aircraftCommand.header.subtype = 0x01;

	// 3. Send data expecting position/velocity response
	if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
		cout << "Error while sending the message for aircraft "  << ": " << strerror(errno) << endl;
		name_close(server_coid);
	}

	return aircraftCommand;
}


Radar::~Radar() {
}
