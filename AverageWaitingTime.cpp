#include "AverageWaitingTime.h"
#include <cmath>

AverageWaitingTime::AverageWaitingTime() {
    gamma = 0.0;
    reqCountConsideredByGamma = 0;
    u = 0.0;
    s = 0.0;
    gammaWithWave = 0.0;
    sWithWave = 0.0;
    streamStatus = StreamStatus_IsStabilizing;
}

void AverageWaitingTime::setStreamStatus(StreamStatus streamStatus_) {
    streamStatus = streamStatus_;
}

double AverageWaitingTime::getGamma() {
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

void AverageWaitingTime::calculateAvgWaitTime(double inputTime, double outputTime) {
    if (reqCountConsideredByGamma == 0) {
        // Go here if it`s first request

        // Calclulate new value of waiting time
        gamma = outputTime - inputTime;

        // Calculate the second power of gamma
        u = gamma * gamma;

        // Increase requests count that have been considered by gamma
        reqCountConsideredByGamma++;

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

void AverageWaitingTime::calculateNewGammaAndUValue(double inputTime, double outputTime) {
    double fraction = 1.0 / (static_cast<double>(reqCountConsideredByGamma) + 1.0);
    double waitingTime = outputTime - inputTime;

    // Compute new value of average waiting time (gamma)
    gamma =
        gamma * fraction * static_cast<double>(reqCountConsideredByGamma) + waitingTime * fraction;

    // Compute new value of u (additional variable for calculating)
    u = u * fraction * static_cast<double>(reqCountConsideredByGamma) +
        waitingTime * waitingTime * fraction;

    // Increase requests count that have been considered by gamma
    reqCountConsideredByGamma++;
}

void AverageWaitingTime::checkErrorForGammaAndS() {
    double fraction = static_cast<double>(reqCountConsideredByGamma) /
                      (static_cast<double>(reqCountConsideredByGamma) - 1.0);

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
