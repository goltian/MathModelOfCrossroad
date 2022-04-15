#pragma once

#include "CarsStream.h"
#include "PeopleStream.h"

class ServiceDevice {
public:

	ServiceDevice(
		std::vector<double> parametersOfSystem,
		std::vector<double> modesDurations
	);

	bool isCrossroadModelWorksStably();

	std::vector<double> getPortionOfData();

	enum Modes {
		Mode_InvalidFirst = -1,
		Mode_First = 0,
		Mode_Gamma1 = 0,
		Mode_Gamma2 = 1,
		Mode_Gamma3 = 2,
		Mode_Gamma4 = 3,
		Mode_Gamma5 = 4,
		Mode_Gamma6 = 5,
		Mode_Gamma7 = 6,
		Mode_Gamma8 = 7,
		Mode_Last = 7
	};

private:
	
	Modes currentMode;

	CarsStream streamPi1;
	CarsStream streamPi2;
	PeopleStream streamPi3;

	struct SwitchesCount {
		int IntoGamma1;
		int IntoGamma3;
		int IntoGamma5;
		int IntoGamma7;
	} switchesCount;

	// Minimal count of requests that have to be in front of crossroad
	// for changing to mode of thier service 
	int minReqCountToStartService;

	// Percent of switching service device into gamma5 and gamma7 modes
	double percentOfSwitchingIntoG5AndG7;
	
	// Get average waiting time of all streams
	double getAvgGammaForAllStreams();

	double getAvgReqCountInBunkerForAllStreams();

	double getAvgDowntimeForAllStreams();

	double getPercentOfSwitchingIntoG5AndG7();

	void computeNextMode();

	void generateRequestsForAll();

	void changeModeDurationForAll();

};
