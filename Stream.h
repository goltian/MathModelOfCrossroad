#pragma once

#include <random>
#include <queue>
#include <random>
#include "AverageWaitingTime.h"
#include "mkl.h"

constexpr float CONST_EXPON_PUAS_AND_BART = 1e-32F;

constexpr float CONST_EPS_COMPARISON = 1e-6F;

constexpr uint16_t CONST_FOR_SLOW_REQ_COUNT = 150;

constexpr int CONST_CRITICAL_REQ_COUNT = 1000;

constexpr uint16_t CONST_SIZE_OF_RAND_VALUES_VECTOR = 1024;

class Stream {
public:

	Stream();

	~Stream();

	void setG(float g_);

	void setMathExpect(float mathExpect_);

	void setLiam(float liam_);

	void setModesDurations(std::vector<float> modesDuration_);

	void calculateR();

	void calculateLiamBartlett();

	float getLiam();

	float getLiamBartlett();

	uint16_t getStorageBunkerSize();

	float getGamma();

	bool isStreamNotStable();

	bool isStreamStable();

	// Method for calculating the array of the exponents for every mode
	void calculateExponents();

	// Method for calculating massives of Poisson distriution for every mode
    void calculatePoissonDist();

	void changeModeDuration(int modeId);

	void generateRequests(int modeId);

	virtual void serviseRequests() = 0;

	float getAvgReqCountInBunker();

protected:

	std::mt19937 generator;
    std::uniform_int_distribution<int> distribution;

    MKL_INT method;

    VSLStreamStatePtr cur_stream;


	// Vector of random values. We will fill many values at one time
	// to use it by one in the future
    float* randomValues;

	uint16_t countOfUsedRandValues;

	// Parameters of stream
	float g;
	float mathExpect;
	float r;
	float liam;
	float liamBartlett;

	// Total time of working crossroad
	float totalTime;
	
	// Duration of the mode
	float modeDuration;

	// Real size of queue in storage bunker
    uint16_t reqCountInBunker;

	// Pointer to start of queue in storage bunker
	int pointerToStartOfBunker;

	// Pointer to end of queue in storage bunker
	int pointerToEndOfBunker;
	
	// Durations of every mode
	std::vector<float> modesDuration;
	
	// Array of the calculated exponents for every mode
	std::vector<float> exponents;

	// Arrays of the calculated Poisson distibution for every mode
	std::vector<std::vector<float>> poissonDistribution;

	// Vector of times of incoming requests that have
    // got into storage bunker
    std::vector<float> storageBunker;
	
	// Array for times of coming slow requests that have 
	// come during last interval (mode)
	std::vector <float> timesOfSlowReq;

	// Max possible count of requests that 
	// could be on a crossroad at one time
    uint16_t throughputCapacity;
	
	// Requests count that could be served 
	// if their count was infinite
    uint16_t reqCountOfSaturation;
	
	// Service time for one request
	float serviceTime;

	AverageWaitingTime avgWaitingTime;

	// Method for generating count of slow requests on interval
    uint16_t generatePoisson(int modeId);
	
	// Method for generating count of fast requests in every bundle
	float generateBartlett();
	
	// Method for checking the need to generate fast requests at all
	bool areThereFastRequests();

	// Method for calculating requests count that
	// could be served if their count was infinite
	void calculateReqCountOfSaturation();

	// Method for filling massive of random values for using
	// these values in the future
	void fillVectorOfRandValues();

	// Number of activating of service modes
    float activateServiceModesCount;
	
    // The average requests count in storage bunker
    float avgReqCountInBunker; 

	// Method for updating the average requests count in storage bunker
    void updateTheAvgReqCountInBunker(); 
	
	void insertionSort(uint16_t slowReqCount);

};
