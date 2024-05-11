/*
 * Aircraft.cpp
 *
 *  Created on: Feb. 10, 2024
 *      Author: David Molina
 */

#include "Aircraft.h"
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <sys/dispatch.h>
#include <vector>
#include <string>
#include "cTimer.h"

using namespace std;

#define ATTACH_POINT "default"

Aircraft::Aircraft(){
	//default constructor
	this->thread = 0;
}

Aircraft::Aircraft(int flightId, int positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time){
	this->thread = 0;

	this->flightId = flightId;

	this->positionX = positionX;
	this->positionY = positionY;
	this->positionZ = positionZ;

	this->speedX = speedX;
	this->speedY = speedY;
	this->speedZ = speedZ;

	this->time = time;

	err_no = pthread_attr_init(&attr);
	if (err_no!=0){
		printf("ERROR from pthread_attr_init() is %d \n", err_no);
	}
	err_no = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (err_no!=0){
		printf("ERROR from pthread_attr_setdetachstate() is %d \n", err_no);
	}
	err_no= pthread_attr_setstacksize(&attr, 256);
	if (err_no!=0){
		printf("ERROR from pthread_attr_setstacksize() is %d \n", err_no);
	}

}


// aircraftListenHelper function, used to unpack arguments and call listen with attachPoint
void* Aircraft::aircraftListenHelper(void* args) {
	AircraftListenArgs* arguments = (AircraftListenArgs*)args;
	return arguments->aircraft->listen(arguments->attachPoint);
}

Aircraft::~Aircraft() {

}

int Aircraft::getFlightId(){
	return flightId;
}

int Aircraft::getPositionX(){
	return positionX;
}

int Aircraft::getPositionY(){
	return positionY;
}

int Aircraft::getPositionZ(){
	return positionZ;
}

int Aircraft::getPrevPositionX(){
	return positionX-speedX;
}

int Aircraft::getPrevPositionY(){
	return positionY-speedY;
}

int Aircraft::getPrevPositionZ(){
	return positionZ-speedZ;
}

int Aircraft::getSpeedX(){
	return speedX;
}

int Aircraft::getSpeedY(){
	return speedY;
}

int Aircraft::getSpeedZ(){
	return speedZ;
}

void Aircraft::setFlightId(int flightId){
	this->flightId = flightId;
}

void Aircraft::setPositionX(int positionX){
	this->positionX = positionX;
}

void Aircraft::setPositionY(int positionY){
	this->positionY = positionY;
}

void Aircraft::setPositionZ(int positionZ){
	this->positionZ = positionZ;
}

void Aircraft::setSpeedX(int speedX){
	this->speedX = speedX;
}

void Aircraft::setSpeedY(int speedY){
	this->speedY = speedY;
}

void Aircraft::setSpeedZ(int positionZ){
	this->speedZ = speedZ;
}


void Aircraft::startThread(){
	err_no = pthread_create(&thread, &attr, &Aircraft::planeOperations, this);
	if (err_no != 0){
		printf("ERROR when creating thread number: %d \n", err_no);
	}


	//err_no = pthread_join(thread,NULL);
	if (err_no != 0){
		printf("ERROR when joining thread number: %d \n", err_no);
	}
}

void Aircraft::stopThread(){
	err_no = pthread_cancel(thread);
	if (err_no != 0){
		printf("ERROR when cancelling thread number: %d \n", err_no);
	}
}

void *Aircraft::planeOperations(void *arg) {
	Aircraft* a = ((Aircraft*)arg);
	cTimer timer(1,0);//1 second, 0 milliseconds interval

	while(true){
		a->setPositionX(a->getPositionX() + a->getSpeedX());
		a->setPositionY(a->getPositionY() + a->getSpeedY());
		a->setPositionZ(a->getPositionZ() + a->getSpeedZ());
		//printf("Flight Id: %d, New location: X=%d, Y=%d, Z=%d\n", a->getFlightId(), a->getPositionX(), a->getPositionY(), a->getPositionZ());
		timer.waitTimer();
	}

	return NULL;
}


void* Aircraft::listen(string attachPoint) {

	name_attach_t *attach;

	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		cout << "Aircraft (listen): Error occurred while creating the attach point." << endl;
	}

	AircraftData aircraftCommand;

	// listen infinitely for commands received
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

		if (aircraftCommand.header.type == 0x00) {
			if (aircraftCommand.header.subtype == 0x01) {
				// Send aircraft position and velocity (0x00 and 0x01)
				aircraftCommand.flightId = flightId;
				aircraftCommand.positionX = positionX;
				aircraftCommand.positionY = positionY;
				aircraftCommand.positionZ = positionZ;
				aircraftCommand.speedX = speedX;
				aircraftCommand.speedY = speedY;
				aircraftCommand.speedZ = speedZ;
				MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));
				break;
				//continue;
			} else if (aircraftCommand.header.subtype == 0x02) {
				// Update current aircraft speed (0x00 and 0x02)
				speedX = aircraftCommand.speedX;
				speedY = aircraftCommand.speedY;
				speedZ = aircraftCommand.speedZ;

				MsgReply(rcvid, EOK, NULL, 0);
				break;
			} else {
				MsgError(rcvid, ENOSYS);
				continue;
			}
		}
	}

	/* Remove the name from the space */
	name_detach(attach, 0);

	return NULL;
}
