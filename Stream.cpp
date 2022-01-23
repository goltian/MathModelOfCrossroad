#include <algorithm>
#include "Stream.h"
#include <random>

Stream::Stream() {
    //std::random_device device;
    //generator.seed(device());
    //vslNewStream(&cur_stream, VSL_BRNG_MT19937, distribution(generator));

	// Initialize MKL Stream for generating random values
    vslNewStream(&cur_stream, VSL_BRNG_MT19937, 0);
    method = VSL_RNG_METHOD_UNIFORM_STD;

	randomValues = (float*)mkl_malloc((CONST_SIZE_OF_RAND_VALUES_VECTOR) * sizeof(float), 64);
	// Fill vector of random values for using in future
    fillVectorOfRandValues();

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

	// Use vector instead of queue to have sequential memory access
	// Max size of queue could be 1000
	storageBunker.resize(CONST_CRITICAL_REQ_COUNT + 1);

    pointerToStartOfBunker = 0;
    pointerToEndOfBunker = 0;
    reqCountInBunker = 0;

	avgReqCountInBunker = 0;
    activateServiceModesCount = 0;
}

Stream::~Stream() {
    vslDeleteStream(&cur_stream);
    mkl_free(randomValues);
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

    for (size_t i = 0; i < modesCount; ++i) {
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

uint16_t Stream::getStorageBunkerSize() {
    return reqCountInBunker;
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
    uint16_t modesCount = static_cast<int>(modesDuration.size());
    poissonDistribution.resize(modesCount);
    float modeDur;
    float adding;

    for (uint16_t modeId = 0; modeId < modesCount; ++modeId) {
        modeDur = modesDuration[modeId];
        adding = 1.0F;
        poissonDistribution[modeId].resize(CONST_FOR_SLOW_REQ_COUNT);
        poissonDistribution[modeId][0] = 1.0F;

        for (uint16_t curValue = 1; curValue < CONST_FOR_SLOW_REQ_COUNT; ++curValue) {
			// Calculate next value of Poisson dist
            adding *= liamBartlett * modeDur / curValue;
            poissonDistribution[modeId][curValue] =
                poissonDistribution[modeId][curValue - 1] + adding;

			// If we try to increase last value with zero then leave this method
            if (adding < CONST_EXPON_PUAS_AND_BART) {
                poissonDistribution[modeId][curValue + 1] = FLT_MAX;
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

    // Number of slow requests
    uint16_t slowReqCount;

    // Generate count of slow requests on the interval
    slowReqCount = generatePoisson(modeId);
	// Increase req count that will be in bunker
    reqCountInBunker += slowReqCount;

    // Cycle for putting arriving times of slow requests into the array
    for (uint16_t slowReq = 0; slowReq < slowReqCount; ++slowReq) {
        // Generate random value from 0 to 1

        // Calculate the time of arriving a slow request
        //timesOfSlowReq[slowReq] = modeDuration * distribution(generator) + totalTime;
		timesOfSlowReq[slowReq] = modeDuration * randomValues[countOfUsedRandValues++] + totalTime;
        if ( (CONST_SIZE_OF_RAND_VALUES_VECTOR - 1) < countOfUsedRandValues ) {
            fillVectorOfRandValues();
		}
    }

    // We need to sort our array
    if (slowReqCount > 2) {
	    std::sort(timesOfSlowReq.begin(), timesOfSlowReq.begin() + slowReqCount);
    } else if (slowReqCount == 2) {
        if (timesOfSlowReq[0] > timesOfSlowReq[1]) {
            std::swap(timesOfSlowReq[0], timesOfSlowReq[1]);
		}
	}

    // If there are fast requests then we need generate them
    if (areThereFastRequests()) {
        // Number of fast requests for every bundle
        float fastReqCount;

        // Distance (time) between fast requests
        float timeBetweenFastReq;

        // Cycle for filling fast and slow requests into queue
        for (uint16_t slowReq = 0; slowReq < slowReqCount; ++slowReq) {
            fastReqCount = generateBartlett();

            // Put slow request into queue
			// We can rewrite old times in our vector because in that case req count in bunker
			// will be over 1000 and we will go out from programm with "not stable stream"
            storageBunker[pointerToEndOfBunker] = timesOfSlowReq[slowReq];
            if (pointerToEndOfBunker < CONST_CRITICAL_REQ_COUNT) {
                pointerToEndOfBunker = pointerToEndOfBunker + 1;
            } else {
                pointerToEndOfBunker = 0;
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
            for (uint16_t fastReq = 0; fastReq < static_cast<uint16_t>(fastReqCount); ++fastReq) {
                fastReqInFloat += 1.0F;

                // We can rewrite old times in our vector because in that case req count in bunker
                // will be over 1000 and we will go out from programm with "not stable stream"
                storageBunker[pointerToEndOfBunker] =
                    timesOfSlowReq[slowReq] + fastReqInFloat * timeBetweenFastReq;
                if (pointerToEndOfBunker < CONST_CRITICAL_REQ_COUNT) {
                    pointerToEndOfBunker = pointerToEndOfBunker + 1;
                } else {
                    pointerToEndOfBunker = 0;
                }
            }
            // Increase req count that will be in bunker
            reqCountInBunker += static_cast<uint16_t>(fastReqCount);
        }
    } else {
        // If there is no fast requests put all slow requests into queue
        for (uint16_t slowReq = 0; slowReq < slowReqCount; ++slowReq) {
            // We can rewrite old times in our vector because in that case req count in bunker
            // will be over 1000 and we will go out from programm with "not stable stream"
            storageBunker[pointerToEndOfBunker] = timesOfSlowReq[slowReq];
            if (pointerToEndOfBunker < CONST_CRITICAL_REQ_COUNT) {
                pointerToEndOfBunker = pointerToEndOfBunker + 1;
            } else {
                pointerToEndOfBunker = 0;
            }
        }
    }

    totalTime += modeDuration;
}

float Stream::getAvgReqCountInBunker() {
    return avgReqCountInBunker;
}

uint16_t Stream::generatePoisson(int modeId) {
	
    // The value of a random variable that the distribution function should approach
    float randomVariable;

    // Counter for slow requests
    uint16_t slowReqCount = 0;

    // Generate random value from 0 to 1
    randomVariable = randomValues[countOfUsedRandValues++];
    if ((CONST_SIZE_OF_RAND_VALUES_VECTOR - 1) < countOfUsedRandValues) {
        fillVectorOfRandValues();
    }

    // Multiply to exponent (putting it out of the bracket)
    randomVariable = randomVariable * exponents[modeId];

	// Calculate number of slow requests
    while (poissonDistribution[modeId][slowReqCount] < randomVariable) {
        ++slowReqCount;
    }

    return slowReqCount;
}

float Stream::generateBartlett() {
    // The value of a random variable that the distribution function should approach
    float randomVariable;

    // The value of a distribution function
    float distributionFunc = 1.0F - r;

    // Counter for fast requests in a bundle
    uint16_t fastReqCount = 0;

    // Flag for checking that we have passed first iteration successfully
    bool isFirstIterPassed = true;

    // Adding for distribution function
    float adding = r * (1.0F - g);

    // Generate random value from 0 to 1
    randomVariable = randomValues[countOfUsedRandValues++];
    if ((CONST_SIZE_OF_RAND_VALUES_VECTOR - 1) < countOfUsedRandValues) {
        fillVectorOfRandValues();
    }

    // Make compare first time
    if (distributionFunc >= randomVariable) {
        isFirstIterPassed = false;
    }

    if (isFirstIterPassed) {
        // Calculate number of fast requests
        while (distributionFunc < randomVariable) {
            ++fastReqCount;
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
    reqCountOfSaturation = static_cast<uint16_t>(throughputCapacity) *
                           static_cast<uint16_t>(modeDuration) / static_cast<uint16_t>(serviceTime);
}

inline void Stream::fillVectorOfRandValues() {
	vsRngUniform(method, cur_stream, CONST_SIZE_OF_RAND_VALUES_VECTOR, randomValues, 0.0F, 1.0F);
	countOfUsedRandValues = 0;
}

void Stream::updateTheAvgReqCountInBunker() {
	// Get requests count in storage bunker
    float countInBunker = static_cast<float>(getStorageBunkerSize());

	// Recalculate the average requests count in bunker
    avgReqCountInBunker = (avgReqCountInBunker * activateServiceModesCount + countInBunker) /
                       (activateServiceModesCount + 1.0F);
    activateServiceModesCount += 1.0F;
}
