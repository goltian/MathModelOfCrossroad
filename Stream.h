#pragma once

#include <random>
#include <queue>
#include "AverageWaitingTime.h"
#include <boost/random.hpp>

constexpr auto CONST_EXPON_PUAS_AND_BART = 1e-32;

constexpr auto CONST_EPS_COMPARISON = 1e-6;

constexpr auto CONST_FOR_SLOW_REQ_COUNT = 150;

constexpr auto CONST_CRITICAL_REQ_COUNT = 1000;

class Stream {
public:

	Stream();

	void setG(double g_);

	void setMathExpect(double mathExpect_);

	void setLiam(double liam_);

	void setModesDurations(std::vector<int> modesDuration_);

	void calculateR();

	void calculateLiamBartlett();

	double getLiam();

	double getLiamBartlett();

	int getStorageBunkerSize();

	double getGamma();

	bool isStreamNotStable();

	bool isStreamStable();

	// Method for calculating the array of the exponents for every mode
	void calculateExponents();

	void changeModeDuration(int modeId);

	void generateRequests(int modeId);

	virtual void serviseRequests() = 0;

protected:

	boost::random::mt19937 generator;
    boost::random::uniform_real_distribution<> distribution;

	// Parameters of stream
	double g;
	double mathExpect;
	double r;
	double liam;
	double liamBartlett;

	// Total time of working crossroad
	int totalTime;
	
	// Duration of the mode
	int modeDuration;
	
	// Durations of every mode
	std::vector<int> modesDuration;
	
	// Array of the calculated exponents for every mode
	std::vector<double> exponents;
	
	// Queue of times of incoming requests that have 
	// got into storage bunker
	std::queue <double> storageBunker;
	
	// Array for times of coming slow requests that have 
	// come during last interval (mode)
	std::vector <double> timesOfSlowReq;

	// Max possible count of requests that 
	// could be on a crossroad at one time
	size_t throughputCapacity;
	
	// Requests count that could be served 
	// if their count was infinite
	int reqCountOfSaturation;
	
	// Service time for one request
	int serviceTime;

	AverageWaitingTime avgWaitingTime;

	// Method for generating count of slow requests on interval
	int generatePuasson(int modeId);
	
	// Method for generating count of fast requests in every bundle
	int generateBartlett();
	
	// Method for checking the need to generate fast requests at all
	bool areThereFastRequests();

	// Method for calculating requests count that
	// could be served if their count was infinite
	void calculateReqCountOfSaturation();
};
