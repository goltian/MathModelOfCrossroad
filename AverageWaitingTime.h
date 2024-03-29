#pragma once

#include <vector>

constexpr double CONST_EPS_FOR_CHECKING_STABLE = 1.0;

constexpr uint16_t CONST_SIZE_TIMES_IN_SYSTEM_VECTOR = 1000;

constexpr double CONST_SIZE_TIMES_IN_SYSTEM_VECTOR_IN_DOUBLE = 1000.0;

class AverageWaitingTime {
public:
	AverageWaitingTime();

	enum StreamStatus {
		StreamStatus_InvalidFirst = -1,
		StreamStatus_First = 0,
		StreamStatus_IsStabilizing = 0,
		StreamStatus_Stable = 1,
		StreamStatus_NotStable = 2,
		StreamStatus_Last = 2
	};

	void setStreamStatus(StreamStatus streamStatus_);

	double getGamma();

	uint16_t getCounter();

	int getReqCountConsideredByGamma();

	bool isStreamStatusNotStable();

	bool isStreamStatusStable();

	// Method for calculating average waiting time of cars
	void calculateAvgWaitTime(double inputTime, double outputTime);

private:
    uint16_t counter;
	std::vector<double> timesInSystemOfRequests;

	// Variable for counting average stay time a request in system
	double gamma;
	
	// Count of requests that has been considered by gamma 
	int reqCountConsideredByGamma;
	
	// Variable for calculating the second power of stay time a request in system
	double u;
	
	// Variable for calculating the estimate of dispersion of stay time a request in system
	double s;
	
	// Additional gamma variable with a wave for counting average 
	// stay time a request in system (used for comparing with gamma)
	double gammaWithWave;
	
	// Additional s variable with a wave for calculating the estimate of dispersion of
	// stay time a request in system (used for comparing with s)
	double sWithWave;
	
	// Flag for exit from programm (0 - continue, 1 - success end, 2 - fail end)
	StreamStatus streamStatus;

	// Method for calculating new value for gamma and u
	void calculateNewGammaAndUValue();
	
	// Method for checking error for gamma and s
	void checkErrorForGammaAndS();
};
