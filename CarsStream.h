#pragma once

#include "Stream.h"

class CarsStream : public Stream {
public:

	CarsStream();

	void serviseRequests();

	float calculateOutputTime(float inputTime, float outputTime, float timeBeforeStartThisMode);

};
