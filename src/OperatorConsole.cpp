/*
 * OperatorConsole.cpp
 *
 *  Created on: Feb. 11, 2024
 *      Author: Alexandre
 */

#include "OperatorConsole.h"
#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Aircraft.h"

#include <sys/dispatch.h>
#include <vector>
#include <string>

#define ATTACH_POINT "default"

using namespace std;

OperatorConsole::OperatorConsole() {
}

void OperatorConsole::writeLog(string log_entry){
	int code = creat("/data/home/qnxuser/OperatorLog.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if(code==-1){
			std::cout<<"Operator commands could not be logged, error"<<endl;
		}else{
			//The below is done to create a buffer to write to a text file according to QNX libraries
			char *blockBuffer = new char[log_entry.length()+1];
			sprintf(blockBuffer, "%s", log_entry.c_str());
			write(code, blockBuffer, log_entry.length()+1);
			write(code, "\n", 1);
			delete[] blockBuffer;
			close(code);
		}
}
void* OperatorConsole::listenForOperatorInput(void* args) {
	OperatorConsole* operatorConsole = (OperatorConsole*)args;
	operatorConsole->listenForUserInput();
	return NULL;
}

void* OperatorConsole::updateAircraftSpeedThread(void* args) {
	OperatorConsole* operatorConsole = (OperatorConsole*)args;
	operatorConsole->updateAircraftSpeed();
	return NULL;
}

void* OperatorConsole::displayAircraftDataThread(void* args) {
	OperatorConsole* operatorConsole = (OperatorConsole*)args;
	operatorConsole->displayAircraftData();
	return NULL;
}

/**
 * Listen For User Input
 */
void OperatorConsole::listenForUserInput() {

	// Continuously listen for user input
	while (true) {
		int commandType;
		int flightId;


		// Prompt the user to enter flight ID and speeds
		cout << endl << "Enter your choice... (1 to display an aircraft's info, 2 to update an aircraft's speed, 3 for separation constraint)" << endl << endl << endl << endl;
		cin >> commandType;

		// Display Aircraft Info
        if (commandType == 1) {
        	cout << endl << "Enter Flight Id: ";
			cin >> flightId;
        	log_entry +="display flight info for flight #";
        	log_entry += std::to_string(flightId);
        	writeLog(log_entry);
        	log_entry="";
        	pthread_t displayDataThread;
            pthread_create(&displayDataThread, nullptr, &OperatorConsole::displayAircraftDataThread, this);

			AircraftData aircraftCommand;
			aircraftCommand.header.type = 0x04;
			aircraftCommand.header.subtype = 0x02;
			aircraftCommand.flightId = flightId;

			// sleep, otherwise error will be thrown when creating/attaching relevant channels
			sleep(1);

			// Generate the unique attach point name
			string attachPoint = string(ATTACH_POINT) + "info";

			// establish connection (communication)
			int coid;
			if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
				perror("Error occurred while attaching the channel listenForUserInput FUNCTION [1]");
				return;
			}

			// Send data expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
				name_close(coid);
				return;
			}

		// Update Aircraft Speed
		} else if (commandType == 2) {
			cout << endl << "Enter Flight Id: ";
			cin >> flightId;
        	log_entry +="Change flight speed for flight #";
        	log_entry += std::to_string(flightId);
        	log_entry += "\nNew speeds (X,Y,Z) = (";
			pthread_t updateAircraftSpeedThread;
			pthread_create(&updateAircraftSpeedThread, nullptr, &OperatorConsole::updateAircraftSpeedThread, this);

			int newSpeedX, newSpeedY, newSpeedZ;

			// Get speeds from operator input
			cout << endl << "Enter SpeedX: ";
			cin >> newSpeedX;
            log_entry += std::to_string(newSpeedX);
            log_entry += ",";
			cout << endl << "Enter SpeedY: ";
			cin >> newSpeedY;
            log_entry += std::to_string(newSpeedY);
            log_entry += ",";
			cout << endl << "Enter SpeedZ: ";
			cin >> newSpeedZ;
            log_entry += std::to_string(newSpeedZ);
            log_entry += ")";
            writeLog(log_entry);
            log_entry="";

			AircraftData aircraftCommand;
			aircraftCommand.header.type = 0x04;
			aircraftCommand.header.subtype = 0x01;
			aircraftCommand.flightId = flightId;
			aircraftCommand.speedX = newSpeedX;
			aircraftCommand.speedY = newSpeedY;
			aircraftCommand.speedZ = newSpeedZ;

			// Generate the unique attach point name
			string attachPoint = string(ATTACH_POINT) + "inner_transfer";

			// establish connection (communication)
			int coid;
			if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
				perror("Error occurred while attaching the channel listenForUserInput FUNCTION [2]");
				return;
			}

			// Send data expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
				name_close(coid);
				return;
			}


		// Separation constraint
		}else if(commandType == 3){
			log_entry +="Change input N  and interval P at runtime for separation constraint \n";
			log_entry +="New input N and P: ";
			// Get n and p from operator input
			int n;
			cout << endl << "Enter N seconds for separation constraint: ";
			cin >> n;
			log_entry += n + ", ";
			int p;
			cout << endl << "Enter interval for separation constraint: ";
			cin >> p;
			log_entry += p + "\n";
			// Generate the unique attach point name
			string attachPoint = string(ATTACH_POINT) + "_separation";

			// establish connection (communication)
			int coid;
			if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
				perror("Error occurred while attaching the channel listenForUserInput FUNCTION [3]");
				return;
			}
			SeparationData separationCommand;
			separationCommand.n_seconds = n;
			separationCommand.p_interval = p;

			// Send data expecting no response
			if (MsgSend(coid, &separationCommand, sizeof(separationCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for computer system: " << strerror(errno) << endl;
				name_close(coid);
				return;
			}
		}else {
			cout << "Invalid command type." << endl;
		}


	}
}


void* OperatorConsole::updateAircraftSpeed() {

	name_attach_t *attach;

	// Generate the unique attach point name
	string attachPointInner = string(ATTACH_POINT) + "inner_transfer";

	if ((attach = name_attach(NULL, attachPointInner.c_str(), 0)) == NULL) {
		perror("Error occurred while creating the attach point updateAircraftSpeed");
		return (void*)EXIT_FAILURE;
	}

	AircraftData aircraftCommand;
	while (true) {
		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

		if (rcvid == -1) { // Error condition, exit
			break;
		}

		if (aircraftCommand.header.type == 0x04 && aircraftCommand.header.subtype == 0x01) {
			MsgReply(rcvid, EOK, NULL, 0);
			name_detach(attach, 0);

			// establish connection (communication)
			int coid;
			if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
				perror("Error occurred while attaching the channel UPDATEAIRCRAFTSPEED FUNCTION");
				return (void*)EXIT_FAILURE;
			}

			aircraftCommand.header.type = 0x02;
			aircraftCommand.header.subtype = 0x00;

			// Send data expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft CSOP "  << ": " << strerror(errno) << endl;
				name_close(coid);
				return (void*)EXIT_FAILURE;
			}

			break;
		}

	}

	return EXIT_SUCCESS;
}

void* OperatorConsole::displayAircraftData() {

	name_attach_t *attach;

	// Generate the unique attach point name
	string attachPointInner = string(ATTACH_POINT) + "info";

	if ((attach = name_attach(NULL, attachPointInner.c_str(), 0)) == NULL) {
		perror("Error occurred while creating the attach point displayAircraftData");
		return (void*)EXIT_FAILURE;
	}

	AircraftData aircraftCommand;
	while (true) {
		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

		if (rcvid == -1) { // Error condition, exit
			break;
		}

		if (aircraftCommand.header.type == 0x04 && aircraftCommand.header.subtype == 0x02) {

			// establish connection (communication)
			int coid;
			if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
				perror("Error occurred while attaching the channel UPDATEAIRCRAFTSPEED FUNCTION");
				return (void*)EXIT_FAILURE;
			}

			aircraftCommand.header.type = 0x02;
			aircraftCommand.header.subtype = 0x01;

			// Send data expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft CSOP "  << ": " << strerror(errno) << endl;
				name_close(coid);
				return (void*)EXIT_FAILURE;
			}

			MsgReply(rcvid, EOK, NULL, 0);
			name_detach(attach, 0);

			break;
		}

	}

	return EXIT_SUCCESS;
}

OperatorConsole::~OperatorConsole() {
}

