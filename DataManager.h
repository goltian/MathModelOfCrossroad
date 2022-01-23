#pragma once

#include <vector>

constexpr int CONST_CELLS_COUNT = 9;

class DataManager {
public:

	DataManager(
		int rowCount_,
		int repeatsCount_
	);
	
	// Save gamma for all streams and avg gamma
	// and percent of switching into g5 and g7
	void setPortionOfData(
		int row, 
		int column, 
		std::vector<float> portionOfData, 
		int tid
	);

	// Write all data into file
	void writeInfoInFile(
		float peopleServiceModeDuration, 
		float liam, 
		std::string nameOfFile, 
		float timeOfWork
	);

	// Write all data into table
	void writeInfoInTable(std::string nameOfFile);

private:

	int rowCount;

	int sizeOfVector;

	int repeatsCount;

	int minValueIndex;

	int successfulRepeatsCount;

	// Min (best) durations for g1 and g3 modes
	std::pair<int, int> bestDurationsOfModes;

	// All data
	std::vector<float> data;

	// Result data
	std::vector<float> resultMatrix;

	// Method that find index in our matrix
	int findIndex(int i, int j);
	
	// Method for calculating size of our matrix
	int calculateSizeOfVector();

	// Method to find out count of
	// succsess repeates of one experiment
    void findCountOfSuccsessRepeats(int index);

	// Set part of avg data into cell of result matrix
	void setAvgData(int index);

	// Method for computing result matrix
	void computeResultMatrix();

	// Method for fining index for best g1 and g3 modes durations
	void findMinValueIndex();

	// Method for replacing point to comma and saving 
	// two places to the right of the decimal point
	std::string replacePointToComma(std::string str);
};
