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
    if (reqCountConsideredByGamma == 0) {
        // Go here if it`s first request

        // Calclulate new value of waiting time
        gamma = outputTime - inputTime;

        // Calculate the second power of gamma
        u = gamma * gamma;

        // Increase requests count that have been considered by gamma
        ++reqCountConsideredByGamma;

    } else if ((reqCountConsideredByGamma % 1000) != 0) {
        // Go here to process every thousand requests
        calculateNewGammaAndUValue(inputTime, outputTime);

    } else if ((reqCountConsideredByGamma % 1000) == 0) {
        // Go here if we have processed another one thousand requests

        // Check if stream have stabilised
        checkErrorForGammaAndS();

        // Don`t forget the value of gamma, save it in gammaWithWave.
        // We will be compare it with new value of gamma
        gammaWithWave = gamma;

        // Don`t forget the value of s, save it in sWithWave.
        // We will be compare it with new value of s
        sWithWave = s;

        calculateNewGammaAndUValue(inputTime, outputTime);
    }
}

void AverageWaitingTime::calculateNewGammaAndUValue(float inputTime, float outputTime) {
    float reqCount = static_cast<float>(reqCountConsideredByGamma);
    float fraction = 1.0F / (reqCount + 1.0F);
    float waitingTime = outputTime - inputTime;

    // Compute new value of average waiting time (gamma)
    gamma = gamma * fraction * reqCount + waitingTime * fraction;

    // Compute new value of u (additional variable for calculating)
    u = u * fraction * reqCount + waitingTime * waitingTime * fraction;

    // Increase requests count that have been considered by gamma
    ++reqCountConsideredByGamma;
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
