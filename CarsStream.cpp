#include "CarsStream.h"

CarsStream::CarsStream() {
    throughputCapacity = 1;
    serviceTime = 1.0F;
}

void CarsStream::serviseRequests() {
    float timeBeforeStartThisMode = totalTime - modeDuration;
    calculateReqCountOfSaturation();

	int reqCountInBunkerBeforeService = reqCountInBunker;

	if (reqCountInBunkerBeforeService > CONST_CRITICAL_REQ_COUNT) {
        avgWaitingTime.setStreamStatus(avgWaitingTime.StreamStatus_NotStable);
        return;
    }


    int reqCountOfServed = 0;
    float inputTime = 0.0F;
    float outputTime = 0.0F;

    while ((reqCountOfServed < reqCountOfSaturation) && (reqCountOfServed < reqCountInBunkerBeforeService)) {
        // Take first request from queue
        inputTime = storageBunker[pointerToStartOfBunker];
		// We service one request. Move pointer
        if (pointerToStartOfBunker < CONST_CRITICAL_REQ_COUNT) {
            ++pointerToStartOfBunker;
        } else {
            pointerToStartOfBunker = 0;
        }

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

        ++reqCountOfServed;
    }
	// Decrease req count in bunker
    reqCountInBunker -= reqCountOfServed;
}
