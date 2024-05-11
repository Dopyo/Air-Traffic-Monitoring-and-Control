#include <iostream>

#include "Aircraft.h"
#include "Radar.h"
#include "CommunicationSystem.h"
#include "DataDisplay.h"
#include "ComputerSystem.h"
#include "OperatorConsole.h"
#include <vector>
#include <fstream>

#define rows 25
#define columns 25

#define ATTACH_POINT "default"


using namespace std;

void createInput(){//call createInput() if you need to modify the input file
	fstream file;
	file.open("low.txt", ios::out); //relative path (you can find the file in /data/var/tmp in the virtual machine)
	if(file.is_open()){
		//int flightId, positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time
		file << "3, 0, 0, 0, 250, 250, 0, 0 \n";
		file << "4, 5000, 0, 0, 250, 0, 0, 0 \n";
		file << "5, 1800, 0, 0, 251, 0, 0, 0 \n";
		//
		// add more aircrafts if needed
		//
		file.close();
	}

	file.open("med.txt", ios::out); //relative path (you can find the file in /data/var/tmp in the virtual machine)
	if(file.is_open()){
		//int flightId, positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time
		file << "3, 0, 0, 0, 250, 250, 0, 0 \n";
		file << "4, 5000, 0, 0, 250, 0, 0, 0 \n";
		file << "5, 1800, 0, 0, 251, 0, 0, 0 \n";
		file << "6, 0, 700, 0, 300, 0, 0, 0 \n";
		file << "7, 1000, 3000, 0, 500, 0, 0, 0 \n";

		//
		// add more aircrafts if needed
		//
		file.close();
	}

	file.open("high.txt", ios::out); //relative path (you can find the file in /data/var/tmp in the virtual machine)
	if(file.is_open()){
		//int flightId, positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time
		file << "3, 0, 0, 0, 250, 250, 0, 0 \n";
		file << "4, 5000, 0, 0, 250, 0, 0, 0 \n";
		file << "5, 1800, 0, 0, 251, 0, 0, 0 \n";
		file << "6, 0, 700, 0, 300, 0, 0, 0 \n";
		file << "7, 1000, 3000, 0, 500, 0, 0, 0 \n";
		file << "8, 4000, 700, 0, 300, 0, 0, 0 \n";
		file << "9, 1000, 200, 0, 500, 0, 0, 0 \n";
		//
		// add more aircrafts if needed
		//
		file.close();
	}
}



void printInput(string filename){//call printInput() if you need to verify that the input file was created
	fstream file;
	file.open(filename, ios::in);//change based on filename
	if(file.is_open()){
		string line;
		while(getline(file, line)){
			cout << line << endl;
		}
		file.close();
	}
}


int main() {
	string filename = "";
	int congestion = 0;
	cout << "type 1 for low, 2 for med, 3 for high congestion" << endl;
	cin >> congestion;
	switch(congestion){
	case 1:
		filename = "low.txt";
		break;
	case 2:
		filename = "med.txt";
		break;
	case 3:
		filename = "high.txt";
		break;
	}


	createInput();
	printInput(filename);

	DataDisplay dataDisplay;
	ComputerSystem computerSystem;
	OperatorConsole operatorConsole;
	Radar radar;
	CommunicationSystem communicationSystem;

	vector<Aircraft*> aircrafts;

	//int flightId, positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time

	/*
		Aircraft aircraft1 = Aircraft(3, 0, 0, 0, 250, 250, 0, 0);
		Aircraft aircraft2 = Aircraft(4, 5000, 0, 0, 250, 0, 0, 0);
		Aircraft aircraft3 = Aircraft(5, 1800, 0, 0, 251, 0, 0, 0);

		//aircraft1.startThread();
		//aircraft2.startThread();
		//aircraft3.startThread();

		aircrafts.push_back(&aircraft1);
		aircrafts.push_back(&aircraft2);
		aircrafts.push_back(&aircraft3);
	*/


	fstream file;
	file.open(filename, ios::in);
	if(file.is_open()){
		string line;
		while(getline(file, line)){

			string delimiter = ",";
			size_t pos = 0;
			string token;
			vector<int> arg;


			while ((pos = line.find(delimiter)) < 10000000) {
			    token = line.substr(0, pos);
			    arg.push_back((int)atoi(token.c_str()));
			    line.erase(0, pos + delimiter.length());

			}
			arg.push_back((int)atoi(line.c_str()));

			aircrafts.push_back(new Aircraft(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7]));
		}
		file.close();
	}

	computerSystem.setAircrafts(aircrafts);
	computerSystem.alarm();
	for(Aircraft* ap : aircrafts){
		ap->startThread();
	}

	// Start the threads
	pthread_t mapDisplayThread, userInputThread, computerSystemThread, dataDisplayThread, mapInitializeThread, separationThread, radarThread;
	pthread_create(&radarThread, nullptr, &Radar::radarInitializer, &radar);
	pthread_create(&mapInitializeThread, nullptr, &DataDisplay::MapInitializer, &dataDisplay);

	pthread_create(&mapDisplayThread, nullptr, &ComputerSystem::MapDisplayThread, &computerSystem);

	pthread_create(&dataDisplayThread, nullptr, &DataDisplay::dataDisplayInitializer, &dataDisplay);
	pthread_create(&computerSystemThread, nullptr, &ComputerSystem::computerSystemThread, &computerSystem);
	pthread_create(&userInputThread, nullptr, &OperatorConsole::listenForOperatorInput, &operatorConsole);
	pthread_create(&separationThread, nullptr, &ComputerSystem::separationCheckThread, &computerSystem);

	// Run infinitely, otherwise main will complete its execution before the threads and program will terminate
	while (true) {

		// reduce resource use
		sleep(500);
	}


	for (size_t i = 0; i < aircrafts.size(); ++i) {
		aircrafts[i]->stopThread();
	}

	return 0;
}

