#pragma once

constexpr float CONST_EPS_FOR_CHECKING_STABLE = 1.0F;

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

	float getGamma();

	bool isStreamStatusNotStable();

	bool isStreamStatusStable();

	// Method for calculating average waiting time of cars
	void calculateAvgWaitTime(float inputTime, float outputTime);

private:
	// Variable for counting average stay time a request in system
	float gamma;
	
	// Count of requests that has been considered by gamma 
	int reqCountConsideredByGamma;
	
	// Variable for calculating the second power of stay time a request in system
	float u;
	
	// Variable for calculating the estimate of dispersion of stay time a request in system
	float s;
	
	// Additional gamma variable with a wave for counting average 
	// stay time a request in system (used for comparing with gamma)
	float gammaWithWave;
	
	// Additional s variable with a wave for calculating the estimate of dispersion of
	// stay time a request in system (used for comparing with s)
	float sWithWave;
	
	// Flag for exit from programm (0 - continue, 1 - success end, 2 - fail end)
	StreamStatus streamStatus;

	// Method for calculating new value for gamma and u
	void calculateNewGammaAndUValue(float inputTime, float outputTime);
	
	// Method for checking error for gamma and s
	void checkErrorForGammaAndS();
};
