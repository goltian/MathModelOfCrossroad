#include "CarsStream.h"

CarsStream::CarsStream() {
	throughputCapacity = 1;
	serviceTime = 1;
}

void CarsStream::serviseRequests() {
	double timeBeforeStartThisMode = static_cast<double>(totalTime -  modeDuration);
	calculateReqCountOfSaturation();
	size_t reqCountInBunker = storageBunker.size();
	int reqCountOfServed = 0;
	double inputTime = 0;
	double outputTime = 0;

	while ((reqCountOfServed < reqCountOfSaturation) && (reqCountOfServed < reqCountInBunker)) {
		// Take first request from queue
		inputTime = storageBunker.front();
		storageBunker.pop();

		// Compute the output time for another one request
		if (inputTime < timeBeforeStartThisMode) {
			// Case in which request have got into bunker before current mode starting
			
			outputTime = std::max(timeBeforeStartThisMode, outputTime) + serviceTime;
		} else if (inputTime <= timeBeforeStartThisMode + modeDuration - serviceTime) {
			// Case in which request have got into bunker after current mode starting
			
			outputTime = std::max(inputTime, outputTime) + serviceTime;
		} else {
			// Case in which current mode finishing and the request is accelerating
			// and does`t waiting another requests
			
			outputTime = inputTime + serviceTime;
		}
		avgWaitingTime.calculateAvgWaitTime(inputTime, outputTime);

		reqCountOfServed++;
	}

	if (storageBunker.size() > CONST_CRITICAL_REQ_COUNT) {
		avgWaitingTime.setStreamStatus(avgWaitingTime.StreamStatus_NotStable);
	}
}
