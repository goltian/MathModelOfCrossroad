#pragma once

#include <random>
#include <queue>
#include <random>
#include "AverageWaitingTime.h"

constexpr float CONST_EXPON_PUAS_AND_BART = 1e-32F;

constexpr float CONST_EPS_COMPARISON = 1e-6F;

constexpr int CONST_FOR_SLOW_REQ_COUNT = 150;

constexpr int CONST_CRITICAL_REQ_COUNT = 1000;

class Stream {
public:

	Stream();

	void setG(float g_);

	void setMathExpect(float mathExpect_);

	void setLiam(float liam_);

	void setModesDurations(std::vector<float> modesDuration_);

	void calculateR();

	void calculateLiamBartlett();

	float getLiam();

	float getLiamBartlett();

	int getStorageBunkerSize();

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

protected:

	std::mt19937 generator;
    std::uniform_real_distribution<float> distribution;

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

	int countInBunker;

	int pointerToStartOfBunker;

	int pointerToEndOfBunker;

	int sizeOfBunker;
	
	// Durations of every mode
	std::vector<float> modesDuration;
	
	// Array of the calculated exponents for every mode
	std::vector<float> exponents;

	// Arrays of the calculated Poisson distibution for every mode
	std::vector<std::vector<float>> poissonDistribution;
		
	//// Queue of times of incoming requests that have 
	//// got into storage bunker
	//std::queue<float> storageBunker;

	// Queue of times of incoming requests that have
    // got into storage bunker
    std::vector<float> storageBunker;
	
	// Array for times of coming slow requests that have 
	// come during last interval (mode)
	std::vector <float> timesOfSlowReq;

	// Max possible count of requests that 
	// could be on a crossroad at one time
	size_t throughputCapacity;
	
	// Requests count that could be served 
	// if their count was infinite
	int reqCountOfSaturation;
	
	// Service time for one request
	float serviceTime;

	AverageWaitingTime avgWaitingTime;

	// Method for generating count of slow requests on interval
	int generatePoisson(int modeId);
	
	// Method for generating count of fast requests in every bundle
	float generateBartlett();
	
	// Method for checking the need to generate fast requests at all
	bool areThereFastRequests();

	// Method for calculating requests count that
	// could be served if their count was infinite
	void calculateReqCountOfSaturation();
};
