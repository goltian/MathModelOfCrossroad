#include "PeopleStream.h"
#include <iostream>

PeopleStream::PeopleStream() {
    throughputCapacity = 5;
    serviceTime = 10.0F;
    criticalTimeForPeople = 7.0F;
    casesInServiceRequests = Case_OldReqWillBeServedNow;
}

void PeopleStream::serviseRequests() {
    totalTime -= modeDuration;
    calculateReqCountOfSaturation();

    int reqCountInBunkerBeforeService = reqCountInBunker;

	if (reqCountInBunkerBeforeService > CONST_CRITICAL_REQ_COUNT) {
        avgWaitingTime.setStreamStatus(avgWaitingTime.StreamStatus_NotStable);
        return;
	}

    int reqCountOfServed = 0;
    float inputTime = 0.0F;
    float outputTime = 0.0F;
    std::queue<float> reqOutputTimes;
    int maxPossibleReqCountToServe = std::min(reqCountOfSaturation, reqCountInBunkerBeforeService);

    while (reqCountOfServed < maxPossibleReqCountToServe) {

        // Take first request from queue
        inputTime = storageBunker[pointerToStartOfBunker];

        // Compute the output time for another one request
        if ((inputTime < totalTime) && (reqOutputTimes.size() < throughputCapacity)) {
            // Case in which request have got into bunker before
            // current mode starting and it can be served at present
            casesInServiceRequests = Case_OldReqWillBeServedNow;

            outputTime = calculateOutputTime(casesInServiceRequests, reqOutputTimes, inputTime);

            reqOutputTimes.push(outputTime);

        } else if ((inputTime < totalTime) && (throughputCapacity < reqOutputTimes.size())) {
            // Case in which request have got into bunker before
            // current mode starting ant it can`t be served at present
            casesInServiceRequests = Case_OldReqCanNotBeServedNow;

            outputTime = calculateOutputTime(casesInServiceRequests, reqOutputTimes, inputTime);

            while (!reqOutputTimes.empty()) {
                reqOutputTimes.pop();
            }

            reqOutputTimes.push(outputTime);

        } else if (reqOutputTimes.size() < throughputCapacity) {
            // Case in which request have got into bunker after
            // current mode starting ant it can be served at present
            casesInServiceRequests = Case_NewReqWillBeServedNow;

            outputTime = calculateOutputTime(casesInServiceRequests, reqOutputTimes, inputTime);

            if (isRequestCantBeServedAtAll(outputTime)) {
                break;
            }

            reqOutputTimes.push(outputTime);

        } else {
            // Case in which request have got into bunker after
            // current mode starting ant it can`t be served at present
            casesInServiceRequests = Case_NewReqCanNotBeServedNow;

            while ((0 < reqOutputTimes.size()) && (reqOutputTimes.size() >= throughputCapacity)) {
                outputTime = calculateOutputTime(casesInServiceRequests, reqOutputTimes, inputTime);
                reqOutputTimes.pop();
            }

            if (isRequestCantBeServedAtAll(outputTime)) {
                break;
            }

            reqOutputTimes.push(outputTime);
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
                                         const std::queue<float> &reqOutputTimes,
                                         float inputTime) {
    float outputTime;

    switch (casesInServiceRequests) {
        case (Case_OldReqWillBeServedNow):

            if (!reqOutputTimes.empty()) {
                float sum = static_cast<float>(totalTime + serviceTime);
                outputTime = std::max(sum, reqOutputTimes.front());
            } else {
                outputTime = totalTime + serviceTime;
            }
            break;

        case (Case_OldReqCanNotBeServedNow):

            outputTime = reqOutputTimes.front() + serviceTime;
            break;

        case (Case_NewReqWillBeServedNow):

            if (!reqOutputTimes.empty()) {
                outputTime = std::max(inputTime + serviceTime, reqOutputTimes.front());
            } else {
                outputTime = inputTime + serviceTime;
            }
            break;

        case (Case_NewReqCanNotBeServedNow):

            outputTime = std::max(inputTime, reqOutputTimes.front()) + serviceTime;
            break;

        default:
            std::cout << "Method 'calculateOutputTime' is not working correctly\n";
            exit(0);
    }

    return outputTime;
}
