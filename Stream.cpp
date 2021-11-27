#include <algorithm>
#include "Stream.h"
#include <random>

Stream::Stream() {
    //std::random_device device;
    //generator.seed(device());
    generator.seed(0);
    g = 0.0;
    mathExpect = 1.0;
    r = (1.0 - g) * (mathExpect - 1.0);
    liam = 0.0;
    liamBartlett = liam / mathExpect;
    totalTime = 0.0;
    modeDuration = 0.0;
    exponents.resize(8);

    // Use resizing here for not resizing
    // every time calling method of generating requests
    timesOfSlowReq.resize(CONST_FOR_SLOW_REQ_COUNT);
    throughputCapacity = 0;
    reqCountOfSaturation = 0;
}

void Stream::setG(double g_) {
    g = g_;
}

void Stream::setMathExpect(double mathExpect_) {
    mathExpect = mathExpect_;
}

void Stream::setLiam(double liam_) {
    liam = liam_;
}

void Stream::setModesDurations(std::vector<double> modesDuration_) {
    size_t modesCount = modesDuration_.size();
    modesDuration.resize(modesCount);

    for (size_t i = 0; i < modesCount; i++) {
        modesDuration[i] = modesDuration_[i];
    }
}

void Stream::calculateR() {
    r = (1.0 - g) * (mathExpect - 1.0);
}

void Stream::calculateLiamBartlett() {
    liamBartlett = liam / mathExpect;
}

double Stream::getLiam() {
    return liam;
}

double Stream::getLiamBartlett() {
    return liamBartlett;
}

int Stream::getStorageBunkerSize() {
    return static_cast<int>(storageBunker.size());
}

double Stream::getGamma() {
    return avgWaitingTime.getGamma();
}

bool Stream::isStreamNotStable() {
    return avgWaitingTime.isStreamStatusNotStable();
}

bool Stream::isStreamStable() {
    return avgWaitingTime.isStreamStatusStable();
}

void Stream::calculateExponents() {
    // Mode for first stream
    exponents[0] = exp(liamBartlett * modesDuration[0]);

    // Mode for second stream
    exponents[2] = exp(liamBartlett * modesDuration[2]);

    // 2 modes of orientation after first and second modes
    exponents[1] = exponents[3] = exp(liamBartlett * modesDuration[1]);

    // 2 modes for third stream
    exponents[4] = exponents[6] = exp(liamBartlett * modesDuration[4]);

    // 2 modes of orientation after third mode
    exponents[5] = exponents[7] = exp(liamBartlett * modesDuration[5]);
}

void Stream::changeModeDuration(int modeId) {
    modeDuration = modesDuration[modeId];
}

void Stream::generateRequests(int modeId) {
    // The value of a random variable that the distribution function should approach
    double randomVariable;

    // Number of slow requests
    int slowReqCount;

    // Generate count of slow requests on the interval
    slowReqCount = generatePuasson(modeId);

    if (slowReqCount > CONST_FOR_SLOW_REQ_COUNT) {
        timesOfSlowReq.resize(slowReqCount);
    }

    // Cycle for putting arriving times of slow requests into the array
    for (int slowReq = 0; slowReq < slowReqCount; slowReq++) {
        // Generate random value from 0 to 1
        randomVariable = distribution(generator);

        // Calculate the time of arriving a slow request
        timesOfSlowReq[slowReq] = totalTime + modeDuration * randomVariable;
    }

    // We need to sort our array
    std::sort(timesOfSlowReq.begin(), timesOfSlowReq.begin() + slowReqCount);

    // If there are fast requests then we need generate them
    if (areThereFastRequests()) {
        // Number of fast requests for every bundle
        int fastReqCount;

        // Distance (time) between fast requests
        double timeBetweenFastReq;

        // Cycle for filling fast and slow requests into queue
        for (int slowReq = 0; slowReq < slowReqCount; slowReq++) {
            fastReqCount = generateBartlett();

            // Put slow request into queue
            storageBunker.push(timesOfSlowReq[slowReq]);

            // Calculate time (distance) between fast requests
            if (slowReq < slowReqCount - 1) {
                timeBetweenFastReq = (timesOfSlowReq[slowReq + 1] - timesOfSlowReq[slowReq]) /
                                     (2.0 * static_cast<double>(fastReqCount));
            } else {
                timeBetweenFastReq = (totalTime + modeDuration - timesOfSlowReq[slowReq]) /
                                     (2.0 * static_cast<double>(fastReqCount));
            }

            // Put fast requests into queue
            for (int fastReq = 0; fastReq < fastReqCount; fastReq++) {
                storageBunker.push(timesOfSlowReq[slowReq] +
                                   (static_cast<double>(fastReq) + 1.0) * timeBetweenFastReq);
            }
        }
    } else {
        // If there is no fast requests put all slow requests into queue
        for (int slowReq = 0; slowReq < slowReqCount; slowReq++) {
            storageBunker.push(timesOfSlowReq[slowReq]);
        }
    }

    totalTime += modeDuration;
}

int Stream::generatePuasson(int modeId) {
	
    // The value of a random variable that the distribution function should approach
    double randomVariable;

    // The value of a distribution function
    double distributionFunc = 1.0;

    // Adding for distribution function
    double adding = 1.0;

    // Counter for slow requests
    int slowReqCount = 0;

    // Generate random value from 0 to 1
    randomVariable = distribution(generator);

    // Multiply to exponent (putting it out of the bracket)
    randomVariable = randomVariable * exponents[modeId];

    // Calculate number of slow requests
    while (distributionFunc < randomVariable) {
        slowReqCount++;
        adding = adding * liamBartlett * modeDuration / static_cast<double>(slowReqCount);
        distributionFunc = distributionFunc + adding;
        if (adding < CONST_EXPON_PUAS_AND_BART) {
            break;
        }
    }

    return slowReqCount;
}

int Stream::generateBartlett() {
    // The value of a random variable that the distribution function should approach
    double randomVariable;

    // The value of a distribution function
    double distributionFunc = 1.0 - r;

    // Counter for fast requests in a bundle
    int fastReqCount = 0;

    // Flag for checking that we have passed first iteration successfully
    bool isFirstIterPassed = true;

    // Adding for distribution function
    double adding = r * (1.0 - g);

    // Generate random value from 0 to 1
    randomVariable = distribution(generator);

    // Make compare first time
    if (distributionFunc >= randomVariable) {
        isFirstIterPassed = false;
    }

    if (isFirstIterPassed) {
        // Calculate number of fast requests
        while (distributionFunc < randomVariable) {
            fastReqCount++;
            distributionFunc += adding;
            adding *= g;
            if (adding < CONST_EXPON_PUAS_AND_BART) {
                break;
            }
        }
    }
    return fastReqCount;
}

bool Stream::areThereFastRequests() {
    return (abs(mathExpect - 1.0) >= CONST_EPS_COMPARISON);
}

void Stream::calculateReqCountOfSaturation() {
    reqCountOfSaturation = static_cast<int>(throughputCapacity) * static_cast<int>(modeDuration) /
                           static_cast<int>(serviceTime);
}
