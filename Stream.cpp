#include <algorithm>
#include <fstream>
#include "Stream.h"

Stream::Stream() {
	std::random_device device;
	generator.seed(device());
	vslNewStream(&curStream, VSL_BRNG_MT19937, distribution(generator));

    // Initialize MKL Stream for generating random values
    // vslNewStream(&curStream, VSL_BRNG_MT19937, 0);
    method = VSL_RNG_METHOD_UNIFORM_STD;

    randomValues = (double*)mkl_malloc((CONST_SIZE_OF_RAND_VALUES_VECTOR) * sizeof(double), 64);
    // Fill vector of random values for using in future
    fillVectorOfRandValues();

    g = 0.0;
    mathExpect = 1.0;

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

    // Use vector instead of queue to have sequential memory access
    // Max size of queue could be 1000
    storageBunker.resize(CONST_CRITICAL_REQ_COUNT + 1);

    pointerToStartOfBunker = 0;
    pointerToEndOfBunker = 0;
    reqCountInBunker = 0;

    avgReqCountInBunker = 0.0;
    avgDowntime = 0.0;
    activateServiceModesCount = 0.0;
    activateAllModesCount = 0.0;

	// Update size of our vector of input and output times for viualisation.
	// We can do visualisation_size equal to max_time because liam < 1
	// and modes duration > 1 sec
    int visualisation_size = static_cast<int>(CONST_MAX_TIME_FOR_VISUALIZATION);
    inputAndOutputTimesVector.resize(visualisation_size);
    inputOutputTimesVectorCounter = 0;

	// Update size of our vector of green light time for viualisation
    greenLightTimeVector.resize(visualisation_size);
    greenLightTimeVectorCounter = 0;
}

Stream::~Stream() {
    vslDeleteStream(&curStream);
    mkl_free(randomValues);
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

    for (size_t i = 0; i < modesCount; ++i) {
        modesDuration[i] = modesDuration_[i];
    }
}

void Stream::calculateR() {
    r = (1.0 - g) * (mathExpect - 1.0);

    if (r > 1.0) {
        std::cout << "r = " << r << "\n";
        std::cout << "Parameters are not correct. r must be less than 1\n";
        exit(0);
    }
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

uint16_t Stream::getStorageBunkerSize() {
    return reqCountInBunker;
}

double Stream::getGamma() {
    return avgWaitingTime.getGamma();
}

int Stream::getReqCountConsideredByGamma() {
    return avgWaitingTime.getReqCountConsideredByGamma();
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
    uint16_t modesCount = static_cast<uint16_t>(modesDuration.size());
    poissonDistribution.resize(modesCount);
    double modeDur;
    double adding;

    for (uint16_t modeId = 0; modeId < modesCount; ++modeId) {
        modeDur = modesDuration[modeId];
        adding = 1.0;
        poissonDistribution[modeId].resize(CONST_FOR_SLOW_REQ_COUNT);
        poissonDistribution[modeId][0] = 1.0;

        for (uint16_t curValue = 1; curValue < CONST_FOR_SLOW_REQ_COUNT; ++curValue) {
            // Calculate next value of Poisson dist
            adding *= liamBartlett * modeDur / curValue;
            poissonDistribution[modeId][curValue] =
                poissonDistribution[modeId][curValue - 1] + adding;

            // If we try to increase last value with zero then leave this method
            if (adding < CONST_EXPON_PUAS_AND_BART) {
                poissonDistribution[modeId][curValue + 1] = DBL_MAX;
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
        timesOfSlowReq[slowReq] = modeDuration * randomValues[countOfUsedRandValues++] +
        totalTime; if ((CONST_SIZE_OF_RAND_VALUES_VECTOR - 1) < countOfUsedRandValues) {
            fillVectorOfRandValues();
        }
    }

    // We need to sort our array
    if (slowReqCount <= 32) {
        insertionSort(slowReqCount);
    } else {
        std::sort(timesOfSlowReq.begin(), timesOfSlowReq.begin() + slowReqCount);
    }

    // If there are fast requests then we need generate them
    if (areThereFastRequests()) {
        // Number of fast requests for every bundle
        double fastReqCount;

        // Distance (time) between fast requests
        double timeBetweenFastReq;

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
                timeBetweenFastReq =
                    (timesOfSlowReq[slowReq + 1] - timesOfSlowReq[slowReq]) / (2.0 * fastReqCount);
            } else {
                timeBetweenFastReq =
                    (totalTime + modeDuration - timesOfSlowReq[slowReq]) / (2.0 * fastReqCount);
            }

            // Put fast requests into queue
            double fastReqInDouble = 0.0;
            for (uint16_t fastReq = 0; fastReq < static_cast<uint16_t>(fastReqCount); ++fastReq) {
                fastReqInDouble += 1.0;

                // We can rewrite old times in our vector because in that case req count in bunker
                // will be over 1000 and we will go out from programm with "not stable stream"
                storageBunker[pointerToEndOfBunker] =
                    timesOfSlowReq[slowReq] + fastReqInDouble * timeBetweenFastReq;
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

	updateTheAvgReqCountInBunkerForAllModes();
    updateActivateAllModesCount();
}

double Stream::getAvgReqCountInBunker() {
    return avgReqCountInBunker;
}

double Stream::getAvgDowntime() {
    return avgDowntime;
}

void Stream::writeInfoForVisualisation(std::string nameOfFile) {
    std::ofstream inputOutputTimeInfo;
    inputOutputTimeInfo.precision(2);
    inputOutputTimeInfo.setf(std::ios::fixed);
    inputOutputTimeInfo.open("../../tables_26.05/eksps_2021_3_potoks/" + nameOfFile + "_io.txt",
                           std::ios::trunc);

	uint16_t counter = 0;
    while (counter + 1 < inputOutputTimesVectorCounter) {
        inputOutputTimeInfo << inputAndOutputTimesVector[counter++] << " ";
        inputOutputTimeInfo << inputAndOutputTimesVector[counter++] << "\n";
    }
    inputOutputTimeInfo.close();

	std::ofstream greenLightTimeInfo;
    greenLightTimeInfo.precision(2);
    greenLightTimeInfo.setf(std::ios::fixed);
    greenLightTimeInfo.open("../../tables_26.05/eksps_2021_3_potoks/" + nameOfFile + "_greenlight.txt",
                             std::ios::trunc);

	counter = 0;
    while (counter + 1 < greenLightTimeVectorCounter) {
        greenLightTimeInfo << greenLightTimeVector[counter++] << " ";
        greenLightTimeInfo << greenLightTimeVector[counter++] << "\n";
    }
    greenLightTimeInfo.close();
}

uint16_t Stream::generatePoisson(int modeId) {

    // The value of a random variable that the distribution function should approach
    double randomVariable;

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

double Stream::generateBartlett() {
    // The value of a random variable that the distribution function should approach
    double randomVariable;

    // The value of a distribution function
    double distributionFunc = 1.0 - r;

    // Counter for fast requests in a bundle
    uint16_t fastReqCount = 0;

    // Flag for checking that we have passed first iteration successfully
    bool isFirstIterPassed = true;

    // Adding for distribution function
    double adding = r * (1.0 - g);

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

    return static_cast<double>(fastReqCount);
}

bool Stream::areThereFastRequests() {
    return (abs(mathExpect - 1.0) >= CONST_EPS_COMPARISON);
}

void Stream::calculateReqCountOfSaturation() {
    reqCountOfSaturation = static_cast<uint16_t>(throughputCapacity) *
                           static_cast<uint16_t>(modeDuration) / static_cast<uint16_t>(serviceTime);
}

inline void Stream::fillVectorOfRandValues() {
	vdRngUniform(method, curStream, CONST_SIZE_OF_RAND_VALUES_VECTOR, randomValues, 0.0, 1.0);
	countOfUsedRandValues = 0;
}


void Stream::updateActivateServiceModesCount() {
    activateServiceModesCount += 1.0;
}

void Stream::updateActivateAllModesCount() {
    activateAllModesCount += 1.0;
}

void Stream::updateTheAvgReqCountInBunkerForAllModes() {
    // Get requests count in storage bunker
    double countInBunker = static_cast<double>(getStorageBunkerSize());

    // Recalculate the average requests count in bunker
    avgReqCountInBunker = (avgReqCountInBunker * activateAllModesCount + countInBunker) /
                          (activateAllModesCount + 1.0);
}

void Stream::updateTheAvgDowntime(uint16_t serviceReqCount) {
	// At first we need to know how many batches of requests were served during the mode
	// (If only one request could be on the crossroad at one time
	// then this value is equal to serviceReqCount)
    double serviceBatchCount =
        static_cast<double>(serviceReqCount) / static_cast<double>(throughputCapacity);

	// Calculate absolute downtime
    double curDowntime = modeDuration - serviceBatchCount * serviceTime;
	
	// Calculate relative downtime
    curDowntime /= modeDuration;

	// Recalculate the average downtime
    avgDowntime =
        (avgDowntime * activateServiceModesCount + curDowntime) / (activateServiceModesCount + 1.0);
}

inline void Stream::insertionSort(uint16_t slowReqCount) {
    for (int i = 1; i < slowReqCount; ++i) {
        int j = i;
        while ((j > 0) && (timesOfSlowReq[j - 1] > timesOfSlowReq[j])) {
            std::swap(timesOfSlowReq[j - 1], timesOfSlowReq[j]);
            j--;
        }
    }
}

void Stream::updateInputAndOutputTimesVector(double inputTime, double outputTime) {
    inputAndOutputTimesVector[inputOutputTimesVectorCounter++] = inputTime;
    inputAndOutputTimesVector[inputOutputTimesVectorCounter++] = outputTime;
}

void Stream::updateGreenLightTimeVector() {
    greenLightTimeVector[greenLightTimeVectorCounter++] = totalTime - modeDuration;
    greenLightTimeVector[greenLightTimeVectorCounter++] = totalTime;
}

