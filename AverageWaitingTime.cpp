#include <cmath>
#include "AverageWaitingTime.h"

AverageWaitingTime::AverageWaitingTime() {
    gamma = 0.0;
    reqCountConsideredByGamma = 0;
    u = 0.0;
    s = 0.0;
    gammaWithWave = 0.0;
    sWithWave = 0.0;
    streamStatus = StreamStatus_IsStabilizing;
    timesInSystemOfRequests.resize(CONST_SIZE_TIMES_IN_SYSTEM_VECTOR);
    counter = 0;
}

void AverageWaitingTime::setStreamStatus(StreamStatus streamStatus_) {
    streamStatus = streamStatus_;
}

int AverageWaitingTime::getReqCountConsideredByGamma() {
    return reqCountConsideredByGamma;
}

double AverageWaitingTime::getGamma() {
    return gamma;
}

uint16_t AverageWaitingTime::getCounter() {
    return counter;
}

bool AverageWaitingTime::isStreamStatusNotStable() {
    if (streamStatus == StreamStatus_NotStable) {
        return true;
    } else {
        return false;
    }
}

bool AverageWaitingTime::isStreamStatusStable() {
    if (streamStatus == StreamStatus_Stable) {
        return true;
    } else {
        return false;
    }
}

void AverageWaitingTime::calculateAvgWaitTime(double inputTime, double outputTime) {
    if (counter < CONST_SIZE_TIMES_IN_SYSTEM_VECTOR) {
        // Go here to process every thousand requests
        timesInSystemOfRequests[counter++] = outputTime - inputTime;

    } else if (counter == CONST_SIZE_TIMES_IN_SYSTEM_VECTOR) {
        // Go here if we have processed another one thousand requests

		calculateNewGammaAndUValue();

        // Check if stream have stabilised
        checkErrorForGammaAndS();

        // Don`t forget the value of gamma, save it in gammaWithWave.
        // We will be compare it with new value of gamma
        gammaWithWave = gamma;

        // Don`t forget the value of s, save it in sWithWave.
        // We will be compare it with new value of s
        sWithWave = s;

		counter = 0;
		timesInSystemOfRequests[counter++] = outputTime - inputTime;
    }
}

void AverageWaitingTime::calculateNewGammaAndUValue() {
    double sum = 0;
    double sum_square = 0;
    for (uint16_t i = 0; i < CONST_SIZE_TIMES_IN_SYSTEM_VECTOR; ++i) {
        sum += timesInSystemOfRequests[i];
        sum_square += timesInSystemOfRequests[i] * timesInSystemOfRequests[i];
	}
    double reqCount = static_cast<double>(reqCountConsideredByGamma);
    
	gamma = gamma * reqCount + sum;
    gamma /= (reqCount + CONST_SIZE_TIMES_IN_SYSTEM_VECTOR_IN_DOUBLE);

    u = u * reqCount + sum_square;
    u /= (reqCount + CONST_SIZE_TIMES_IN_SYSTEM_VECTOR_IN_DOUBLE);

    reqCountConsideredByGamma += CONST_SIZE_TIMES_IN_SYSTEM_VECTOR;
}

void AverageWaitingTime::checkErrorForGammaAndS() {
    double reqCount = static_cast<double>(reqCountConsideredByGamma);
    double fraction = reqCount / (reqCount - 1.0);

    // If gamma and s are stable then stream is stable too
    if (abs(gamma - gammaWithWave) < CONST_EPS_FOR_CHECKING_STABLE) {

        // Calculate the estimate of dispersion of stay time a request in system
        s = fraction * (u - gamma * gamma);

        if (abs(s - sWithWave) < CONST_EPS_FOR_CHECKING_STABLE) {
            // Stream is stable
            streamStatus = StreamStatus_Stable;
        }
    }
}
