#include "CarsStream.h"

CarsStream::CarsStream() {
    throughputCapacity = 1;
    serviceTime = 1.0;
}

void CarsStream::serviseRequests() {
    double timeBeforeStartThisMode = totalTime - modeDuration;
    bool TheServiceIsOver = false;

    calculateReqCountOfSaturation();

    uint16_t reqCountInBunkerBeforeService = reqCountInBunker;

    if (reqCountInBunkerBeforeService > CONST_CRITICAL_REQ_COUNT) {
        avgWaitingTime.setStreamStatus(avgWaitingTime.StreamStatus_NotStable);
        return;
    }

    uint16_t reqCountOfServed = std::min(reqCountOfSaturation, reqCountInBunkerBeforeService);
    double inputTime = 0.0;
    double outputTime = 0.0;
    double previosoutputTime = 0.0;

    for (uint16_t curReq = 0; curReq < reqCountOfServed; ++curReq) {
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
        } else if (inputTime <= totalTime - serviceTime) {
            // Case in which request have got into bunker after current mode starting

            outputTime = std::max(inputTime, outputTime) + serviceTime;

			// Current mode is finishing. In this case we can`t serve requests after that one
			if (outputTime > totalTime) {
                TheServiceIsOver = true;
			}

        } else {
            // Case in which current mode is finishing and the request is accelerating
            // and is`t waiting another requests

            outputTime = inputTime + serviceTime;
        }
        avgWaitingTime.calculateAvgWaitTime(inputTime, outputTime);

		previosoutputTime = outputTime;

		if (TheServiceIsOver) {
            reqCountOfServed = curReq + 1;
            break;
		}

    }
    // Decrease req count in bunker
    reqCountInBunker -= reqCountOfServed;

    updateTheAvgReqCountInBunker();
}