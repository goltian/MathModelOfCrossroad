#include "CarsStream.h"

CarsStream::CarsStream() {
    throughputCapacity = 1;
    serviceTime = 1.0F;
}

void CarsStream::serviseRequests() {
    float timeBeforeStartThisMode = totalTime - modeDuration;
    calculateReqCountOfSaturation();

	uint16_t reqCountInBunkerBeforeService = reqCountInBunker;

	if (reqCountInBunkerBeforeService > CONST_CRITICAL_REQ_COUNT) {
        avgWaitingTime.setStreamStatus(avgWaitingTime.StreamStatus_NotStable);
        return;
    }


    uint16_t reqCountOfServed = std::min(reqCountOfSaturation, reqCountInBunkerBeforeService);
    float inputTime = 0.0F;
    float outputTime = 0.0F;

    if ((avgWaitingTime.getCounter() + reqCountInBunker - 1) < CONST_SIZE_TIMES_IN_SYSTEM_VECTOR) {
	
	int newPointer = 0;
    newPointer = (pointerToStartOfBunker + reqCountOfServed) % (CONST_CRITICAL_REQ_COUNT + 1);

	if ((pointerToStartOfBunker + reqCountOfServed - 1) <= CONST_CRITICAL_REQ_COUNT) {
        for (uint16_t curReq = 0; curReq < reqCountOfServed; ++curReq) {
            // Take first request from queue
            inputTime = storageBunker[pointerToStartOfBunker];
            // We service one request. Move pointer
            ++pointerToStartOfBunker;

            outputTime = calculateOutputTime(inputTime, outputTime, timeBeforeStartThisMode);
            avgWaitingTime.calculate(inputTime, outputTime);
        }
    } else {
        for (pointerToStartOfBunker; pointerToStartOfBunker <= CONST_CRITICAL_REQ_COUNT;
             ++pointerToStartOfBunker) {

            // Take first request from queue
            inputTime = storageBunker[pointerToStartOfBunker];

            outputTime = calculateOutputTime(inputTime, outputTime, timeBeforeStartThisMode);
            avgWaitingTime.calculate(inputTime, outputTime);
        }
        pointerToStartOfBunker = 0;
		
		for (pointerToStartOfBunker; pointerToStartOfBunker < newPointer; ++pointerToStartOfBunker) {

            // Take first request from queue
            inputTime = storageBunker[pointerToStartOfBunker];

            outputTime = calculateOutputTime(inputTime, outputTime, timeBeforeStartThisMode);
            avgWaitingTime.calculate(inputTime, outputTime);
        }
	}
    } else {
        int newPointer = 0;
        newPointer = (pointerToStartOfBunker + reqCountOfServed) % (CONST_CRITICAL_REQ_COUNT + 1);

        if ((pointerToStartOfBunker + reqCountOfServed - 1) <= CONST_CRITICAL_REQ_COUNT) {
            for (uint16_t curReq = 0; curReq < reqCountOfServed; ++curReq) {
                // Take first request from queue
                inputTime = storageBunker[pointerToStartOfBunker];
                // We service one request. Move pointer
                ++pointerToStartOfBunker;

				outputTime = calculateOutputTime(inputTime, outputTime, timeBeforeStartThisMode);
                avgWaitingTime.calculateAvgWaitTime(inputTime, outputTime);
            }
        } else {
            for (pointerToStartOfBunker; pointerToStartOfBunker <= CONST_CRITICAL_REQ_COUNT;
                 ++pointerToStartOfBunker) {

                // Take first request from queue
                inputTime = storageBunker[pointerToStartOfBunker];

				outputTime = calculateOutputTime(inputTime, outputTime, timeBeforeStartThisMode);
                avgWaitingTime.calculateAvgWaitTime(inputTime, outputTime);
            }
            pointerToStartOfBunker = 0;

            for (pointerToStartOfBunker; pointerToStartOfBunker < newPointer;
                 ++pointerToStartOfBunker) {

                // Take first request from queue
                inputTime = storageBunker[pointerToStartOfBunker];

				outputTime = calculateOutputTime(inputTime, outputTime, timeBeforeStartThisMode);
                avgWaitingTime.calculateAvgWaitTime(inputTime, outputTime);
            }
        }
	}

  //  for (uint16_t curReq = 0; curReq < reqCountOfServed; ++curReq) {
  //      // Take first request from queue
  //      inputTime = storageBunker[pointerToStartOfBunker];
		//// We service one request. Move pointer
  //      if (pointerToStartOfBunker < CONST_CRITICAL_REQ_COUNT) {
  //          ++pointerToStartOfBunker;
  //      } else {
  //          pointerToStartOfBunker = 0;
  //      }

  //      // Compute the output time for another one request
  //      if (inputTime < timeBeforeStartThisMode) {
  //          // Case in which request have got into bunker before current mode starting

  //          outputTime = std::max(timeBeforeStartThisMode, outputTime) + serviceTime;
  //      } else if (inputTime <= timeBeforeStartThisMode + modeDuration - serviceTime) {
  //          // Case in which request have got into bunker after current mode starting

  //          outputTime = std::max(inputTime, outputTime) + serviceTime;
  //      } else {
  //          // Case in which current mode finishing and the request is accelerating
  //          // and does`t waiting another requests

  //          outputTime = inputTime + serviceTime;
  //      }
  //      avgWaitingTime.calculateAvgWaitTime(inputTime, outputTime);
  //  }
	// Decrease req count in bunker
    reqCountInBunker -= reqCountOfServed;

	updateTheAvgReqCountInBunker();
}

float CarsStream::calculateOutputTime(float inputTime, float outputTime, float timeBeforeStartThisMode) {
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

	return outputTime;
}