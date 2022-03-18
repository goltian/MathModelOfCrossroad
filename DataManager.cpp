#include <iostream>
#include <fstream>
#include <string>
#include "DataManager.h"

DataManager::DataManager(int rowCount_, int repeatsCount_) {
    rowCount = rowCount_;
    repeatsCount = repeatsCount_;
    minValueIndex = -1;
    minQueueValueIndex = -1;

    sizeOfVector = calculateSizeOfVector();
    data.resize(sizeOfVector * CONST_CELLS_COUNT * repeatsCount);

    resultMatrix.resize(sizeOfVector * CONST_CELLS_COUNT, -1);
}

void DataManager::setPortionOfData(int row, int column, std::vector<float> portionOfData,
                                   int tid) {
    int index = findIndex(row - 1, column - 1);
    for (int i = 0; i < CONST_CELLS_COUNT; ++i) {
        data[index + i * sizeOfVector + tid * CONST_CELLS_COUNT * sizeOfVector] = portionOfData[i];
    }
}

void DataManager::writeInfoInFile(float peopleServiceModeDuration, float liam,
                                  std::string nameOfFile, float timeOfWork) {
    std::ofstream reportFile;
    reportFile.precision(2);
    reportFile.setf(std::ios::fixed);
    reportFile.open("../../tables_26.05/eksps_2021_3_potoks/" + nameOfFile + "table.txt",
                    std::ios::trunc);
    std::string str;

    computeResultMatrix();
    findMinValueIndex();
    findMinQueueValueIndex();

    if (minValueIndex == -1) {
        reportFile << "Time of work = " << timeOfWork << "\n";
        reportFile << "Your system not working";
    } else {
        reportFile << "Time of work = " << timeOfWork << "\n";
        reportFile << "cars1 = " << bestDurationsOfModes.first << "\t";
        reportFile << "cars2 = " << bestDurationsOfModes.second << "\n";

		reportFile << "cars1minQueue = " << bestDurationsOfModesByQueueEstimate.first << "\t";
        reportFile << "cars2minQueue = " << bestDurationsOfModesByQueueEstimate.second << "\n";
        reportFile << "people = " << peopleServiceModeDuration << "\t";
        reportFile << "liam = " << liam << "\n";
        for (int repeatNumber = 0; repeatNumber < repeatsCount; ++repeatNumber) {
            for (int cells = 0; cells < CONST_CELLS_COUNT; ++cells) {
                str = std::to_string(data[minValueIndex + cells * sizeOfVector +
                                          repeatNumber * CONST_CELLS_COUNT * sizeOfVector]);
                str = replacePointToComma(str);
                reportFile << str << "\t";
            }

			// Write min queue value of the every repeat
			str = std::to_string(data[minQueueValueIndex + repeatNumber * CONST_CELLS_COUNT * sizeOfVector]);
            str = replacePointToComma(str);
            reportFile << str << "\n";
        }

        for (int cells = 0; cells < CONST_CELLS_COUNT; ++cells) {
            str = std::to_string(resultMatrix[minValueIndex + cells * sizeOfVector]);
            str = replacePointToComma(str);
            reportFile << str << "\t";
        }

        // Write avg min queue value
		str = std::to_string(resultMatrix[minQueueValueIndex]);
        str = replacePointToComma(str);
        reportFile << str;
    }
    reportFile.close();
}

void DataManager::writeInfoInTable(std::string nameOfFile) {
    std::ofstream reportTable;
    reportTable.precision(2);
    reportTable.setf(std::ios::fixed);
    reportTable.open("../../tables_26.05/eksps_2021_3_potoks/" + nameOfFile + "table.xls",
                     std::ios::trunc);
    std::string str;

    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < rowCount; ++column) {
            if (row + column + 2 < rowCount) {
                int index = findIndex(row, column);
                str = std::to_string(resultMatrix[index]);
                str = replacePointToComma(str);
                reportTable << str << "\t";
            }
        }
        reportTable << "\n";
    }
}

void DataManager::writeQueueInfoInTable(std::string nameOfFile) {
    std::ofstream reportTable;
    reportTable.precision(2);
    reportTable.setf(std::ios::fixed);
    reportTable.open("../../tables_26.05/eksps_2021_3_potoks/" + nameOfFile + "queue_table.xls",
                     std::ios::trunc);
    std::string str;

    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < rowCount; ++column) {
            if (row + column + 2 < rowCount) {
                int index = findIndex(row, column) + (CONST_CELLS_COUNT - 1) *sizeOfVector;
                str = std::to_string(resultMatrix[index]);
                str = replacePointToComma(str);
                reportTable << str << "\t";
            }
        }
        reportTable << "\n";
    }
}

void DataManager::writeInfoAboutTheEffectOfParameterN(std::string nameOfFile) {
    std::ofstream reportFile;
    reportFile.precision(2);
    reportFile.setf(std::ios::fixed);
    reportFile.open("../../tables_26.05/eksps_2021_3_potoks/InfoAboutN.txt",
                    std::ios::app);
    std::string str;

    if (minValueIndex == -1) {
        reportFile << "-\t-\t-\n";
    } else {
		// Write minimal value of the avg sojourn (waiting) time
		str = std::to_string(resultMatrix[minValueIndex]);
		str = replacePointToComma(str);
		reportFile << str << "\t";

		// Write queue value with g1 and g3 params 
		// of the minimal value of the avg sojourn (waiting) time
		str = std::to_string(resultMatrix[minValueIndex + (CONST_CELLS_COUNT - 1) * sizeOfVector]);
		str = replacePointToComma(str);
		reportFile << str << "\t";

		// Write value of the avg sojourn (waiting) time with g1 and g3 params
		// of the minimal queue value
        str = std::to_string(
            resultMatrix[minQueueValueIndex - (CONST_CELLS_COUNT - 1) * sizeOfVector]);
        str = replacePointToComma(str);
        reportFile << str << "\t";

		// Write minimal queue value
        str = std::to_string(resultMatrix[minQueueValueIndex]);
        str = replacePointToComma(str);
        reportFile << str << "\t";

		// Write queue value for the first stream
        str = std::to_string(resultMatrix[minValueIndex + (CONST_CELLS_COUNT - 4) * sizeOfVector]);
        str = replacePointToComma(str);
        reportFile << str << "\t";

		// Write queue value for the second stream
        str = std::to_string(resultMatrix[minValueIndex + (CONST_CELLS_COUNT - 3) * sizeOfVector]);
        str = replacePointToComma(str);
        reportFile << str << "\t";

		// Write queue value for the third stream
        str = std::to_string(resultMatrix[minValueIndex + (CONST_CELLS_COUNT - 2) * sizeOfVector]);
        str = replacePointToComma(str);
        reportFile << str << "\n";
	}
}

int DataManager::findIndex(int row, int column) {
    // It always be an integer number because it`s
    // a summ of an arithmetic progression
    float index = (((2 * rowCount - (row - 1)) / 2.0F) * row + column);
    return static_cast<int>(index);
}

int DataManager::calculateSizeOfVector() {
    // It always be an integer number because it`s
    // a summ of an arithmetic progression
    float size = (((2 * rowCount - (rowCount - 2)) / 2.0F) * (rowCount - 1) + 1);
    return static_cast<int>(size);
}

void DataManager::findCountOfSuccsessRepeats(int index) {
    successfulRepeatsCount = 0;
	for (int repeat = 0; repeat < repeatsCount; ++repeat) {
		if (data[index + repeat * sizeOfVector * CONST_CELLS_COUNT] != 0) {
            ++successfulRepeatsCount;
		}
	}
}

void DataManager::setAvgData(int index) {
    float avg = 0.0F;
    float sum = 0.0F;

    for (int cell = 0; cell < CONST_CELLS_COUNT; ++cell) {
        sum = 0.0F;
		// Even if not all repeats were successful we can plus them (plus null)
        for (int repeat = 0; repeat < repeatsCount; ++repeat) {
            sum += data[index + repeat * sizeOfVector * CONST_CELLS_COUNT + cell * sizeOfVector];
        }

		// Keep it in mind that we have to divide by a number of successful repeats
        avg = sum / successfulRepeatsCount;

        resultMatrix[index + cell * sizeOfVector] = avg;
    }
}

void DataManager::computeResultMatrix() {
    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < rowCount; ++column) {
            if (row + column < rowCount) {
                int index = findIndex(row, column);

				findCountOfSuccsessRepeats(index);
                if (successfulRepeatsCount > 0) {
                    setAvgData(index);
                }
            }
        }
    }
}

void DataManager::findMinValueIndex() {
    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < rowCount; ++column) {
            if (row + column < rowCount) {
                int index = findIndex(row, column);

                if (minValueIndex == -1) {
                    if (resultMatrix[index] > 0) {
                        minValueIndex = index;
                        bestDurationsOfModes = std::make_pair(row + 1, column + 1);
                        break;
                    }
                } else if ((resultMatrix[index] > 0) &&
                           (resultMatrix[index] < resultMatrix[minValueIndex])) {
                    minValueIndex = index;
                    bestDurationsOfModes = std::make_pair(row + 1, column + 1);
                }
            }
        }
    }
}

void DataManager::findMinQueueValueIndex() {

    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < rowCount; ++column) {
            if (row + column < rowCount) {
                int index = findIndex(row, column) + (CONST_CELLS_COUNT - 1) * sizeOfVector;
                
                if (minQueueValueIndex == -1) {
                    if (resultMatrix[index] > 0) {
                        minQueueValueIndex = index;
                        bestDurationsOfModesByQueueEstimate = std::make_pair(row + 1, column + 1);
                        break;
                    }
                } else if ((resultMatrix[index] > 0) &&
                           (resultMatrix[index] < resultMatrix[minQueueValueIndex])) {
                    minQueueValueIndex = index;
                    bestDurationsOfModesByQueueEstimate = std::make_pair(row + 1, column + 1);
                }
            }
        }
    }
}

std::string DataManager::replacePointToComma(std::string str) {
    size_t size = str.length();
    size_t indexOfComma = size;
    for (size_t i = 0; i < size; ++i) {
        if (str[i] == '.') {
            str[i] = ',';
            indexOfComma = i;
        }
    }
    for (size_t i = size; i > indexOfComma + 3; i--) {
        str.pop_back();
    }

    return str;
}
