#pragma once

#include <vector>

constexpr int CONST_CELLS_COUNT = 10;

constexpr size_t CONST_DECIMAL_POINTS_SAVE_COUNT = 2;

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
		std::vector<double> portionOfData, 
		int tid
	);

	// Write all data into file
	void writeInfoInFile(
		double peopleServiceModeDuration, 
		double liam, 
		std::string nameOfFile, 
		double timeOfWork
	);

	// Write all data into table
	void writeInfoInTable(std::string nameOfFile);

	void writeQueueInfoInTable(std::string nameOfFile);

	void writeDowntimeInfoInTable(std::string nameOfFile);

	void writeInfoAboutTheEffectOfParameterN(std::string nameOfFile);

private:

	int rowCount;

	int sizeOfVector;

	int repeatsCount;

	int minValueIndex;

	int minQueueValueIndex;

	int successfulRepeatsCount;

	// Min (best) durations for g1 and g3 modes
	std::pair<int, int> bestDurationsOfModes;

	// Best durations for g1 and g3 modes if we find it by minimazing queue value
    std::pair<int, int> bestDurationsOfModesByQueueEstimate;

	// All data
	std::vector<double> data;

	// Result data
	std::vector<double> resultMatrix;

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

	// Method for fining the minimum value of queue in queue table
    void findMinQueueValueIndex();

	// Method for replacing point to comma and 
	// for saving needed places to the right of the decimal point
    std::string replacePointToComma(std::string str,
                                    size_t save_count = CONST_DECIMAL_POINTS_SAVE_COUNT);
};
