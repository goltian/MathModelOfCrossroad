#include <iostream>
#include "PeopleStream.h"

PeopleStream::PeopleStream() {
    throughputCapacity = 5;
    serviceTime = 10.0;
    criticalTimeForPeople = 7.0;
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
	uint16_t maxPossibleReqCountToServe =
        std::min(reqCountOfSaturation, reqCountInBunkerBeforeService);
    double inputTime = 0.0;
    double outputTime = 0.0;

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
            // current mode starting ant it can't be served at present
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
            // current mode starting ant it can't be served at present
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

		// Write info about input and output times for visualisation
		// Need "+ 1" because we write two values
        if (CONST_OF_WRITING_INFO_FOR_VISUALISATION &&
            (inputTime < CONST_MAX_TIME_FOR_VISUALIZATION)) {
            updateInputAndOutputTimesVector(inputTime, outputTime);
		}

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

	updateTheAvgReqCountInBunkerForAllModes();
    updateActivateAllModesCount();

    updateTheAvgDowntime(reqCountOfServed);
    updateActivateServiceModesCount();

	// Write info about green light time for visualisation
    if (CONST_OF_WRITING_INFO_FOR_VISUALISATION &&
		(totalTime < CONST_MAX_TIME_FOR_VISUALIZATION)) {
        updateGreenLightTimeVector();
    }
}

bool PeopleStream::isRequestCantBeServedAtAll(double outputTime) {
    if (totalTime + modeDuration - criticalTimeForPeople < outputTime - serviceTime) {
        return true;
    } else {
        return false;
    }
}

double PeopleStream::calculateOutputTime(CasesInServiceRequests casesInServiceRequests,
                                         double inputTime) {
    double outputTime;

    switch (casesInServiceRequests) {
        case (Case_OldReqWillBeServedNow):

			if (reqCountInOuputQueue > 0) {
                double sum = static_cast<double>(totalTime + serviceTime);
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
