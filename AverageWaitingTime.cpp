#include "AverageWaitingTime.h"
#include <cmath>

AverageWaitingTime::AverageWaitingTime() {
    gamma = 0.0F;
    reqCountConsideredByGamma = 0;
    u = 0.0F;
    s = 0.0F;
    gammaWithWave = 0.0F;
    sWithWave = 0.0F;
    streamStatus = StreamStatus_IsStabilizing;
    timesInSystemOfRequests.resize(1000);
    counter = 0;
}

void AverageWaitingTime::setStreamStatus(StreamStatus streamStatus_) {
    streamStatus = streamStatus_;
}

float AverageWaitingTime::getGamma() {
    return gamma;
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

void AverageWaitingTime::calculateAvgWaitTime(float inputTime, float outputTime) {
    //if (reqCountConsideredByGamma == 0) {
    //    // Go here if it`s first request

    //    // Calclulate new value of waiting time
    //    gamma = outputTime - inputTime;

    //    // Calculate the second power of gamma
    //    u = gamma * gamma;

    //    // Increase requests count that have been considered by gamma
    //    ++reqCountConsideredByGamma;

    //} else if ((reqCountConsideredByGamma % 1000) != 0) {
    if (counter < 1000) {
        // Go here to process every thousand requests
        // calculateNewGammaAndUValue(inputTime, outputTime);
        timesInSystemOfRequests[counter++] = outputTime - inputTime;

    } else if (counter == 1000) {
        // Go here if we have processed another one thousand requests

		calculateNewGammaAndUValue(inputTime, outputTime);

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

        //calculateNewGammaAndUValue(inputTime, outputTime);
    }
}

void AverageWaitingTime::calculateNewGammaAndUValue(float inputTime, float outputTime) {
    float sum = 0;
    for (uint16_t i = 0; i < 1000; ++i) {
        sum += timesInSystemOfRequests[i];
	}
    float reqCount = static_cast<float>(reqCountConsideredByGamma);
    
	gamma = gamma * reqCount + sum;
    gamma /= (reqCount + 1000.0F);

    u = u * reqCount + sum * sum / (1000.0F);
    u /= (reqCount + 1000.0F);

    reqCountConsideredByGamma += 1000;
}

void AverageWaitingTime::checkErrorForGammaAndS() {
    float reqCount = static_cast<float>(reqCountConsideredByGamma);
    float fraction = reqCount / (reqCount - 1.0F);

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
