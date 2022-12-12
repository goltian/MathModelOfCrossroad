#include <iostream>
#include "ServiceDevice.h"

ServiceDevice::ServiceDevice(std::vector<double> parametersOfSystem,
                             std::vector<double> modesDuration) {
    currentMode = Mode_Gamma2;
    switchesCount.IntoGamma1 = 0;
    switchesCount.IntoGamma3 = 0;
    switchesCount.IntoGamma5 = 0;
    switchesCount.IntoGamma7 = 0;

    // g  m  g  m  l  l_p  N
    streamPi1.setG(parametersOfSystem[0]);
    streamPi2.setG(parametersOfSystem[0]);
    streamPi3.setG(parametersOfSystem[2]);

    streamPi1.setMathExpect(parametersOfSystem[1]);
    streamPi2.setMathExpect(parametersOfSystem[1]);
    streamPi3.setMathExpect(parametersOfSystem[3]);

    streamPi1.setLiam(parametersOfSystem[4]);
    streamPi2.setLiam(parametersOfSystem[4]);
    streamPi3.setLiam(parametersOfSystem[5]);

    minReqCountToStartService = static_cast<int>(parametersOfSystem[6]);

    streamPi1.calculateR();
    streamPi2.calculateR();
    streamPi3.calculateR();

    streamPi1.calculateLiamBartlett();
    streamPi2.calculateLiamBartlett();
    streamPi3.calculateLiamBartlett();

    streamPi1.setModesDurations(modesDuration);
    streamPi2.setModesDurations(modesDuration);
    streamPi3.setModesDurations(modesDuration);

    streamPi1.calculateExponents();
    streamPi2.calculateExponents();
    streamPi3.calculateExponents();

	streamPi1.calculatePoissonDist();
    streamPi2.calculatePoissonDist();
    streamPi3.calculatePoissonDist();

    percentOfSwitchingIntoG5AndG7 = 0.0;
}

bool ServiceDevice::isCrossroadModelWorksStably() {
    while (true) {
        if ((streamPi1.isStreamNotStable()) || (streamPi2.isStreamNotStable()) ||
            (streamPi3.isStreamNotStable())) {
            return false;
        }
        if ((streamPi1.isStreamStable()) && (streamPi2.isStreamStable()) &&
            (streamPi3.isStreamStable())) {
            return true;
        }
        computeNextMode();
    }
}

std::vector<double> ServiceDevice::getPortionOfData() {
    std::vector<double> portionOfData(10);
    portionOfData[0] = getAvgGammaForAllStreams();
    portionOfData[1] = streamPi1.getGamma();
    portionOfData[2] = streamPi2.getGamma();
    portionOfData[3] = streamPi3.getGamma();
    portionOfData[4] = getPercentOfSwitchingIntoG5AndG7();
    portionOfData[5] = streamPi1.getAvgReqCountInBunker();
    portionOfData[6] = streamPi2.getAvgReqCountInBunker();
    portionOfData[7] = streamPi3.getAvgReqCountInBunker();
    portionOfData[8] = getAvgReqCountInBunkerForAllStreams();
    portionOfData[9] = getAvgDowntimeForAllStreams();

    return portionOfData;
}

double ServiceDevice::getAvgGammaForAllStreams() {
    double numenator;
    double denumenator;

    numenator = streamPi1.getGamma() * streamPi1.getLiam();
    numenator += streamPi2.getGamma() * streamPi2.getLiam();
    numenator += streamPi3.getGamma() * streamPi3.getLiam();

    denumenator = streamPi1.getLiam() + streamPi2.getLiam() + streamPi3.getLiam();

    return (numenator / denumenator);
}

double ServiceDevice::getAvgReqCountInBunkerForAllStreams() {
	double numenator;
    double denumenator;

    numenator = streamPi1.getAvgReqCountInBunker() * streamPi1.getLiam();
    numenator += streamPi2.getAvgReqCountInBunker() * streamPi2.getLiam();
    numenator += streamPi3.getAvgReqCountInBunker() * streamPi3.getLiam();

    denumenator = streamPi1.getLiam() + streamPi2.getLiam() + streamPi3.getLiam();

    return (numenator / denumenator);
}

double ServiceDevice::getAvgDowntimeForAllStreams() {
    double answer;

    answer = streamPi1.getAvgDowntime() + streamPi2.getAvgDowntime() + streamPi3.getAvgDowntime();

    answer /= 3.0;

    return answer;
}

double ServiceDevice::getPercentOfSwitchingIntoG5AndG7() {
    double numenator;
    double denumenator;
    double percent;

    numenator = switchesCount.IntoGamma5 + switchesCount.IntoGamma7;
    denumenator = numenator + switchesCount.IntoGamma1 + switchesCount.IntoGamma3;

    percent = numenator / denumenator;

    percent *= 100.0;

    return percent;
}

void ServiceDevice::computeNextMode() {
    int reqCountInThirdBunker;

    switch (currentMode) {
        case Mode_Gamma1:
            // Service first stream

            changeModeDurationForAll();
            generateRequestsForAll();
            streamPi1.serviseRequests();
            currentMode = Mode_Gamma2;

            break;
        case Mode_Gamma2:
            // Service no one

            changeModeDurationForAll();
            generateRequestsForAll();

            // If there are any requests in third stream then we need to serve it
            reqCountInThirdBunker = streamPi3.getStorageBunkerSize();

            if (reqCountInThirdBunker > minReqCountToStartService) {
                ++switchesCount.IntoGamma5;
                currentMode = Mode_Gamma5;
            } else {
                ++switchesCount.IntoGamma3;
                currentMode = Mode_Gamma3;
            }

            break;
        case Mode_Gamma3:
            // Service second stream

            changeModeDurationForAll();
            generateRequestsForAll();
            streamPi2.serviseRequests();
            currentMode = Mode_Gamma4;

            break;
        case Mode_Gamma4:
            // Service no one
			changeModeDurationForAll();
            generateRequestsForAll();

            // If there are any requests in third stream then we need to serve it
            reqCountInThirdBunker = streamPi3.getStorageBunkerSize();
            if (reqCountInThirdBunker > minReqCountToStartService) {
                ++switchesCount.IntoGamma7;
                currentMode = Mode_Gamma7;
            } else {
                ++switchesCount.IntoGamma1;
                currentMode = Mode_Gamma1;
            }

            break;
        case Mode_Gamma5:
            // Service thirs stream

            changeModeDurationForAll();
            generateRequestsForAll();
            streamPi3.serviseRequests();
            currentMode = Mode_Gamma6;

            // Service no one
            changeModeDurationForAll();
            generateRequestsForAll();
            currentMode = Mode_Gamma3;

            break;
        case Mode_Gamma7:
            // Service third stream

            changeModeDurationForAll();
            generateRequestsForAll();
            streamPi3.serviseRequests();
            currentMode = Mode_Gamma8;

            // Service no one
            changeModeDurationForAll();
            generateRequestsForAll();
            currentMode = Mode_Gamma1;

            break;
        default:
            std::cout << "Smth wrong in switch!!!";
            exit(0);
    }
}

void ServiceDevice::generateRequestsForAll() {
    streamPi1.generateRequests(currentMode);
    streamPi2.generateRequests(currentMode);
    streamPi3.generateRequests(currentMode);
}

void ServiceDevice::changeModeDurationForAll() {
    streamPi1.changeModeDuration(currentMode);
    streamPi2.changeModeDuration(currentMode);
    streamPi3.changeModeDuration(currentMode);
}
