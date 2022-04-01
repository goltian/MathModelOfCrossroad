#pragma once

#include "Stream.h"

class PeopleStream : public Stream {
public:

	PeopleStream();

	void serviseRequests();

private:
	enum CasesInServiceRequests {
		Case_InvalidFirst = -1,
		Case_First = 0,
		Case_OldReqWillBeServedNow = 0,
		Case_OldReqCanNotBeServedNow = 1,
		Case_NewReqWillBeServedNow = 2,
		Case_NewReqCanNotBeServedNow = 3,
		Case_Last = 3
	} casesInServiceRequests;
	
	double criticalTimeForPeople;

	// Queue of people output times
	// (use vector for sec memory access)
	std::vector<double> reqOutputTimes;

	// Real size of people output times queue
    uint16_t reqCountInOuputQueue;

    // Pointer to start of people output times queue
    int pointerToStartOfOutputQueue;

    // Pointer to end of people output times queue
    int pointerToEndOfOutputQueue;

	bool isRequestCantBeServedAtAll(double outputTime);

	double calculateOutputTime(
		CasesInServiceRequests casesInServiceRequests, double inputTime);
};
