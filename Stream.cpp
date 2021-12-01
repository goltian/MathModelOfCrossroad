#include <algorithm>
#include "Stream.h"
#include <random>

Stream::Stream() {
    //std::random_device device;
    //generator.seed(device());
    generator.seed(0);
    g = 0.0F;
    mathExpect = 1.0F;
    r = (1.0F - g) * (mathExpect - 1.0F);
    liam = 0.0F;
    liamBartlett = liam / mathExpect;
    totalTime = 0.0F;
    modeDuration = 0.0F;
    exponents.resize(8);

    // Use resizing here for not resizing
    // every time calling method of generating requests
    timesOfSlowReq.resize(CONST_FOR_SLOW_REQ_COUNT);
    throughputCapacity = 0;
    reqCountOfSaturation = 0;

	storageBunker.resize(CONST_CRITICAL_REQ_COUNT + 1);
    pointerToStartOfBunker = 0;
    pointerToEndOfBunker = 0;
    sizeOfBunker = CONST_CRITICAL_REQ_COUNT;
    countInBunker = 0;
}

void Stream::setG(float g_) {
    g = g_;
}

void Stream::setMathExpect(float mathExpect_) {
    mathExpect = mathExpect_;
}

void Stream::setLiam(float liam_) {
    liam = liam_;
}

void Stream::setModesDurations(std::vector<float> modesDuration_) {
    size_t modesCount = modesDuration_.size();
    modesDuration.resize(modesCount);

    for (size_t i = 0; i < modesCount; i++) {
        modesDuration[i] = modesDuration_[i];
    }
}

void Stream::calculateR() {
    r = (1.0F - g) * (mathExpect - 1.0F);
}

void Stream::calculateLiamBartlett() {
    liamBartlett = liam / mathExpect;
}

float Stream::getLiam() {
    return liam;
}

float Stream::getLiamBartlett() {
    return liamBartlett;
}

int Stream::getStorageBunkerSize() {
    //return static_cast<int>(storageBunker.size());
    return countInBunker;
}

float Stream::getGamma() {
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

void Stream::calculatePoissonDist() {
    // Calculate massives of Poisson distribution for every mode
    int modes_count = static_cast<int>(modesDuration.size());
    poissonDistribution.resize(modes_count);
    float modeDur;
    float adding;

    for (int j = 0; j < modes_count; ++j) {
        modeDur = modesDuration[j];
        adding = 1.0F;
        poissonDistribution[j].resize(CONST_FOR_SLOW_REQ_COUNT);
        poissonDistribution[j][0] = 1.0F;

        for (int i = 1; i < CONST_FOR_SLOW_REQ_COUNT; ++i) {
			// Calculate next value of Poisson dist
            adding *= liamBartlett * modeDur / i;
            poissonDistribution[j][i] = poissonDistribution[j][i - 1] + adding;

			// If we try to increase last value with zero then leave this method
            if (adding < CONST_EXPON_PUAS_AND_BART) {
                poissonDistribution[j][i + 1] = FLT_MAX;
                break;
            }
        }
    }
}

void Stream::changeModeDuration(int modeId) {
    modeDuration = modesDuration[modeId];
}

void Stream::generateRequests(int modeId) {
    // The value of a random variable that the distribution function should approach
    //float randomVariable;

    // Number of slow requests
    int slowReqCount;

    // Generate count of slow requests on the interval
    slowReqCount = generatePoisson(modeId);

    // Cycle for putting arriving times of slow requests into the array
    for (int slowReq = 0; slowReq < slowReqCount; ++slowReq) {
        // Generate random value from 0 to 1
        //randomVariable = distribution(generator);

        // Calculate the time of arriving a slow request
        timesOfSlowReq[slowReq] = modeDuration * distribution(generator) + totalTime;
    }

    // We need to sort our array
    std::sort(timesOfSlowReq.begin(), timesOfSlowReq.begin() + slowReqCount);

    // If there are fast requests then we need generate them
    if (areThereFastRequests()) {
        // Number of fast requests for every bundle
        float fastReqCount;

        // Distance (time) between fast requests
        float timeBetweenFastReq;

        // Cycle for filling fast and slow requests into queue
        for (int slowReq = 0; slowReq < slowReqCount; slowReq++) {
            fastReqCount = generateBartlett();

            //// Put slow request into queue
            //storageBunker.push(timesOfSlowReq[slowReq]);
            int i = 0;
			if (pointerToEndOfBunker < sizeOfBunker) {
                i = pointerToEndOfBunker + 1;
			}
			if (i != pointerToStartOfBunker) {
				storageBunker[pointerToEndOfBunker] = timesOfSlowReq[slowReq];  
				pointerToEndOfBunker = i;
				countInBunker++;
			} else {
				countInBunker = 1001;
			}

            // Calculate time (distance) between fast requests
            if (slowReq < slowReqCount - 1) {
                timeBetweenFastReq = (timesOfSlowReq[slowReq + 1] - timesOfSlowReq[slowReq]) /
                                     (2.0F * fastReqCount);
            } else {
                timeBetweenFastReq = (totalTime + modeDuration - timesOfSlowReq[slowReq]) /
                                     (2.0F * fastReqCount);
            }

            // Put fast requests into queue
            float fastReqInFloat = 0.0F;
            for (int fastReq = 0; fastReq < fastReqCount; fastReq++) {
                fastReqInFloat += 1.0F;
                //storageBunker.push(timesOfSlowReq[slowReq] + fastReqInFloat * timeBetweenFastReq);

				int i = 0;
                if (pointerToEndOfBunker < sizeOfBunker) {
                    i = pointerToEndOfBunker + 1;
                }
                if (i != pointerToStartOfBunker) {
					storageBunker[pointerToEndOfBunker] =
						timesOfSlowReq[slowReq] + fastReqInFloat * timeBetweenFastReq;
					pointerToEndOfBunker = i;
					countInBunker++;
                } else {
                    countInBunker = 1001;
				}

            }
        }
    } else {
        // If there is no fast requests put all slow requests into queue
        for (int slowReq = 0; slowReq < slowReqCount; slowReq++) {
            //storageBunker.push(timesOfSlowReq[slowReq]);

            int i = 0;
            if (pointerToEndOfBunker < sizeOfBunker) {
                i = pointerToEndOfBunker + 1;
            }
            if (i != pointerToStartOfBunker) {
				storageBunker[pointerToEndOfBunker] = timesOfSlowReq[slowReq];
				pointerToEndOfBunker = i;
				countInBunker++;
			} else {
				countInBunker = 1001;
			}
        }
    }

    totalTime += modeDuration;
}

int Stream::generatePoisson(int modeId) {
	
    // The value of a random variable that the distribution function should approach
    float randomVariable;

    // Counter for slow requests
    int slowReqCount = 0;

    // Generate random value from 0 to 1
    randomVariable = distribution(generator);

    // Multiply to exponent (putting it out of the bracket)
    randomVariable = randomVariable * exponents[modeId];

	// Calculate number of slow requests
    while (poissonDistribution[modeId][slowReqCount] < randomVariable) {
        slowReqCount++;
    }

    return slowReqCount;
}

float Stream::generateBartlett() {
    // The value of a random variable that the distribution function should approach
    float randomVariable;

    // The value of a distribution function
    float distributionFunc = 1.0F - r;

    // Counter for fast requests in a bundle
    int fastReqCount = 0;

    // Flag for checking that we have passed first iteration successfully
    bool isFirstIterPassed = true;

    // Adding for distribution function
    float adding = r * (1.0F - g);

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
    return static_cast<float>(fastReqCount);
}

bool Stream::areThereFastRequests() {
    return (abs(mathExpect - 1.0F) >= CONST_EPS_COMPARISON);
}

void Stream::calculateReqCountOfSaturation() {
    reqCountOfSaturation = static_cast<int>(throughputCapacity) * static_cast<int>(modeDuration) /
                           static_cast<int>(serviceTime);
}
