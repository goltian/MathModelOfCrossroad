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
	
	int criticalTimeForPeople;

	bool isRequestCantBeServedAtAll(double outputTime);

	double calculateOutputTime(
		CasesInServiceRequests casesInServiceRequests, 
		const std::queue<double> &reqOutputTimes,
		double inputTime
	);
};

