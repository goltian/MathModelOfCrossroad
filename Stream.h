#pragma once

#include <iostream>
#include <random>
#include "mkl.h"
#include "AverageWaitingTime.h"

constexpr double CONST_EXPON_PUAS_AND_BART = 1e-16;

constexpr double CONST_EPS_COMPARISON = 1e-6;

constexpr uint16_t CONST_FOR_SLOW_REQ_COUNT = 150;

constexpr int CONST_CRITICAL_REQ_COUNT = 1000;

constexpr uint16_t CONST_SIZE_OF_RAND_VALUES_VECTOR = 1024;

class Stream {
public:

	Stream();

	virtual ~Stream();

	void setG(double g_);

	void setMathExpect(double mathExpect_);

	void setLiam(double liam_);

	void setModesDurations(std::vector<double> modesDuration_);

	void calculateR();

	void calculateLiamBartlett();

	double getLiam();

	double getLiamBartlett();

	uint16_t getStorageBunkerSize();

	double getGamma();

	int getReqCountConsideredByGamma();

	bool isStreamNotStable();

	bool isStreamStable();

	// Method for calculating the array of the exponents for every mode
	void calculateExponents();

	// Method for calculating massives of Poisson distriution for every mode
    void calculatePoissonDist();

	void changeModeDuration(int modeId);

	void generateRequests(int modeId);

	virtual void serviseRequests() = 0;

	double getAvgReqCountInBunker();

protected:

	std::mt19937 generator;
    std::uniform_int_distribution<int> distribution;

    MKL_INT method;

    VSLStreamStatePtr cur_stream;


	// Vector of random values. We will fill many values at one time
	// to use it by one in the future
    double* randomValues;

	uint16_t countOfUsedRandValues;

	// Parameters of stream
	double g;
	double mathExpect;
	double r;
	double liam;
	double liamBartlett;

	// Total time of working crossroad
	double totalTime;
	
	// Duration of the mode
	double modeDuration;

	// Real size of queue in storage bunker
    uint16_t reqCountInBunker;

	// Pointer to start of queue in storage bunker
	int pointerToStartOfBunker;

	// Pointer to end of queue in storage bunker
	int pointerToEndOfBunker;
	
	// Durations of every mode
	std::vector<double> modesDuration;
	
	// Array of the calculated exponents for every mode
	std::vector<double> exponents;

	// Arrays of the calculated Poisson distibution for every mode
	std::vector<std::vector<double>> poissonDistribution;

	// Vector of times of incoming requests that have
    // got into storage bunker
    std::vector<double> storageBunker;
	
	// Array for times of coming slow requests that have 
	// come during last interval (mode)
	std::vector <double> timesOfSlowReq;

	// Max possible count of requests that 
	// could be on a crossroad at one time
    uint16_t throughputCapacity;
	
	// Requests count that could be served 
	// if their count was infinite
    uint16_t reqCountOfSaturation;
	
	// Service time for one request
	double serviceTime;

	AverageWaitingTime avgWaitingTime;

	// Method for generating count of slow requests on interval
    uint16_t generatePoisson(int modeId);
	
	// Method for generating count of fast requests in every bundle
	double generateBartlett();
	
	// Method for checking the need to generate fast requests at all
	bool areThereFastRequests();

	// Method for calculating requests count that
	// could be served if their count was infinite
	void calculateReqCountOfSaturation();

	// Method for filling massive of random values for using
	// these values in the future
	void fillVectorOfRandValues();

	// Number of activating of service modes
    double activateServiceModesCount;
	
    // The average requests count in storage bunker
    double avgReqCountInBunker; 

	// Method for updating the average requests count in storage bunker
    void updateTheAvgReqCountInBunker(); 
	
	void insertionSort(uint16_t slowReqCount);

};
