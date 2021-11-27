#include "PeopleStream.h"
#include <iostream>

PeopleStream::PeopleStream() {
    throughputCapacity = 5;
    serviceTime = 10.0;
    criticalTimeForPeople = 7.0;
    casesInServiceRequests = Case_OldReqWillBeServedNow;
}

void PeopleStream::serviseRequests() {
    totalTime -= modeDuration;
    calculateReqCountOfSaturation();
    int reqCountInBunker = static_cast<int>(storageBunker.size());
    int reqCountOfServed = 0;
    double inputTime = 0.0;
    double outputTime = 0.0;
    std::queue<double> reqOutputTimes;
    int maxPossibleReqCountToServe = std::min(reqCountOfSaturation, reqCountInBunker);

    while (reqCountOfServed < maxPossibleReqCountToServe) {

        // Take first request from queue
        inputTime = storageBunker.front();

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
        storageBunker.pop();

        reqCountOfServed++;
    }

    if (storageBunker.size() > CONST_CRITICAL_REQ_COUNT) {
        avgWaitingTime.setStreamStatus(avgWaitingTime.StreamStatus_NotStable);
    }

    totalTime += modeDuration;
}

bool PeopleStream::isRequestCantBeServedAtAll(double outputTime) {
    if (totalTime + modeDuration - criticalTimeForPeople < outputTime - serviceTime) {
        return true;
    } else {
        return false;
    }
}

double PeopleStream::calculateOutputTime(CasesInServiceRequests casesInServiceRequests,
                                         const std::queue<double> &reqOutputTimes,
                                         double inputTime) {
    double outputTime;

    switch (casesInServiceRequests) {
        case (Case_OldReqWillBeServedNow):

            if (!reqOutputTimes.empty()) {
                double sum = static_cast<double>(totalTime + serviceTime);
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
