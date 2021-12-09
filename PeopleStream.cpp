#include "PeopleStream.h"
#include <iostream>

PeopleStream::PeopleStream() {
    throughputCapacity = 5;
    serviceTime = 10.0F;
    criticalTimeForPeople = 7.0F;
    casesInServiceRequests = Case_OldReqWillBeServedNow;

	// Resize our vector one time for using it in the future
    reqOutputTimes.resize(CONST_CRITICAL_REQ_COUNT + 1);
}

void PeopleStream::serviseRequests() {
    // Our queue have to be new every time
    reqCountInOuputQueue = 0;
    pointerToStartOfOutputQueue = 0;
    pointerToEndOfOutputQueue = 0;

    totalTime -= modeDuration;
    calculateReqCountOfSaturation();

    uint16_t reqCountInBunkerBeforeService = reqCountInBunker;

	if (reqCountInBunkerBeforeService > CONST_CRITICAL_REQ_COUNT) {
        avgWaitingTime.setStreamStatus(avgWaitingTime.StreamStatus_NotStable);
        return;
	}

    uint16_t reqCountOfServed = 0;
    float inputTime = 0.0F;
    float outputTime = 0.0F;
    uint16_t maxPossibleReqCountToServe =
        std::min(reqCountOfSaturation, reqCountInBunkerBeforeService);

    for (uint16_t curReq = 0; curReq < maxPossibleReqCountToServe; ++curReq) {

        // Take first request from queue
        inputTime = storageBunker[pointerToStartOfBunker];

        // Compute the output time for another one request
        if ((inputTime < totalTime) && (reqCountInOuputQueue < throughputCapacity)) {

            // Case in which request have got into bunker before
            // current mode starting and it can be served at present
            casesInServiceRequests = Case_OldReqWillBeServedNow;

            outputTime = calculateOutputTime(casesInServiceRequests, inputTime);

            // Put slow request into queue
            // We can rewrite old times in our vector because in that case req count in bunker
            // will be over 1000 and we will go out from programm with "not stable stream"
            reqOutputTimes[pointerToEndOfOutputQueue] = outputTime;
            if (pointerToEndOfOutputQueue < CONST_CRITICAL_REQ_COUNT) {
                pointerToEndOfOutputQueue = pointerToEndOfOutputQueue + 1;
            } else {
                pointerToEndOfOutputQueue = 0;
            }
            ++reqCountInOuputQueue;

        } else if ((inputTime < totalTime) && (throughputCapacity < reqCountInOuputQueue)) {
            // Case in which request have got into bunker before
            // current mode starting ant it can`t be served at present
            casesInServiceRequests = Case_OldReqCanNotBeServedNow;

            outputTime = calculateOutputTime(casesInServiceRequests, inputTime);

			// Previous queue of people is full. We need to pop all req from queue
			// But we use vector. Just reset it to zero
            pointerToStartOfOutputQueue = 0;
            reqCountInOuputQueue = 0;

			// Put slow request into queue
            // We can rewrite old times in our vector because in that case req count in bunker
            // will be over 1000 and we will go out from programm with "not stable stream"
            reqOutputTimes[pointerToEndOfOutputQueue] = outputTime;
            if (pointerToEndOfOutputQueue < CONST_CRITICAL_REQ_COUNT) {
                pointerToEndOfOutputQueue = pointerToEndOfOutputQueue + 1;
            } else {
                pointerToEndOfOutputQueue = 0;
            }
            ++reqCountInOuputQueue;

        } else if (reqCountInOuputQueue < throughputCapacity) {
            // Case in which request have got into bunker after
            // current mode starting ant it can be served at present
            casesInServiceRequests = Case_NewReqWillBeServedNow;

            outputTime = calculateOutputTime(casesInServiceRequests, inputTime);

            if (isRequestCantBeServedAtAll(outputTime)) {
                break;
            }

            // Put slow request into queue
            // We can rewrite old times in our vector because in that case req count in bunker
            // will be over 1000 and we will go out from programm with "not stable stream"
            reqOutputTimes[pointerToEndOfOutputQueue] = outputTime;
            if (pointerToEndOfOutputQueue < CONST_CRITICAL_REQ_COUNT) {
                pointerToEndOfOutputQueue = pointerToEndOfOutputQueue + 1;
            } else {
                pointerToEndOfOutputQueue = 0;
            }
            ++reqCountInOuputQueue;

        } else {
            // Case in which request have got into bunker after
            // current mode starting ant it can`t be served at present
            casesInServiceRequests = Case_NewReqCanNotBeServedNow;

            while ((0 < reqCountInOuputQueue) && (reqCountInOuputQueue >= throughputCapacity)) {
                outputTime = calculateOutputTime(casesInServiceRequests, inputTime);
                if (pointerToStartOfOutputQueue < CONST_CRITICAL_REQ_COUNT) {
                    ++pointerToStartOfOutputQueue;
                } else {
                    pointerToStartOfOutputQueue = 0;
                }
                --reqCountInOuputQueue;
            }


            if (isRequestCantBeServedAtAll(outputTime)) {
                break;
            }

            // Put slow request into queue
            // We can rewrite old times in our vector because in that case req count in bunker
            // will be over 1000 and we will go out from programm with "not stable stream"
            reqOutputTimes[pointerToEndOfOutputQueue] = outputTime;
            if (pointerToEndOfOutputQueue < CONST_CRITICAL_REQ_COUNT) {
                pointerToEndOfOutputQueue = pointerToEndOfOutputQueue + 1;
            } else {
                pointerToEndOfOutputQueue = 0;
            }
            ++reqCountInOuputQueue;
        }

        avgWaitingTime.calculateAvgWaitTime(inputTime, outputTime);

		// We service one request. Move pointer
        if (pointerToStartOfBunker < CONST_CRITICAL_REQ_COUNT) {
            ++pointerToStartOfBunker;
        } else {
            pointerToStartOfBunker = 0;
        }

		++reqCountOfServed;
    }
    // Decrease req count in bunker
    reqCountInBunker -= reqCountOfServed;

    totalTime += modeDuration;
}

bool PeopleStream::isRequestCantBeServedAtAll(float outputTime) {
    if (totalTime + modeDuration - criticalTimeForPeople < outputTime - serviceTime) {
        return true;
    } else {
        return false;
    }
}

float PeopleStream::calculateOutputTime(CasesInServiceRequests casesInServiceRequests,
                                         float inputTime) {
    float outputTime;

    switch (casesInServiceRequests) {
        case (Case_OldReqWillBeServedNow):

			if (reqCountInOuputQueue > 0) {
                float sum = static_cast<float>(totalTime + serviceTime);
                outputTime = std::max(sum, reqOutputTimes[pointerToStartOfOutputQueue]);
            } else {
                outputTime = totalTime + serviceTime;
            }
            break;

        case (Case_OldReqCanNotBeServedNow):

            outputTime = reqOutputTimes[pointerToStartOfOutputQueue] + serviceTime;
            break;

        case (Case_NewReqWillBeServedNow):

            if (reqCountInOuputQueue > 0) {
                outputTime = std::max(inputTime + serviceTime,
                                      reqOutputTimes[pointerToStartOfOutputQueue]);
            } else {
                outputTime = inputTime + serviceTime;
            }
            break;

        case (Case_NewReqCanNotBeServedNow):

            outputTime =
                std::max(inputTime, reqOutputTimes[pointerToStartOfOutputQueue]) + serviceTime;
            break;

        default:
            std::cout << "Method 'calculateOutputTime' is not working correctly\n";
            exit(0);
    }

    return outputTime;
}
