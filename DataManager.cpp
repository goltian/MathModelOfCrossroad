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

	data.resize(CONST_CELLS_COUNT);
	for (int i = 0; i < CONST_CELLS_COUNT; ++i) {
		data[i].resize(sizeOfVector * repeatsCount);
	}

	resultMatrix.resize(CONST_CELLS_COUNT);
    for (int i = 0; i < CONST_CELLS_COUNT; ++i) {
        resultMatrix[i].resize(sizeOfVector, -1);
    }
}

void DataManager::setPortionOfData(int row, int column, std::vector<double> portionOfData,
                                   int tid) {
    int index = findIndex(row - 1, column - 1);
    for (int i = 0; i < CONST_CELLS_COUNT; ++i) {
        data[i][index + tid * sizeOfVector] = portionOfData[i];
    }
}

void DataManager::writeInfoInFile(double peopleServiceModeDuration, double liam,
                                  std::string nameOfFile, double timeOfWork) {
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
        reportFile << "AvgG"
                   << "\t"
                   << "Gam1"
                   << "\t"
                   << "Gam2"
                   << "\t"
                   << "Gam3"
                   << "\t"
                   << "Percent"
                   << "\t"
                   << "Req1"
                   << "\t"
                   << "Req2"
                   << "\t"
                   << "Req3"
                   << "\t"
                   << "AvgReq"
                   << "\t"
                   << "MinReq"
                   << "\n";
        for (int repeatNumber = 0; repeatNumber < repeatsCount; ++repeatNumber) {
            for (int cell = Cell_First; cell < Cell_Last; ++cell) {
				writePartOfInfo(reportFile, cell, minValueIndex + repeatNumber * sizeOfVector,
                                MatrixOrData_Data, TauOrN_Tau);
            }

            // Write min queue value of the every repeat
            writePartOfInfo(reportFile, Cell_AvgReq,
                            minQueueValueIndex + repeatNumber * sizeOfVector,
                            MatrixOrData_Data, TauOrN_N);
        }

        for (int cell = Cell_First; cell < Cell_Last; ++cell) {
            writePartOfInfo(reportFile, cell, minValueIndex, MatrixOrData_Matrix,
                            TauOrN_Tau);
        }

        // Write avg min queue value
        writePartOfInfo(reportFile, Cell_AvgReq, minQueueValueIndex, MatrixOrData_Matrix,
                        TauOrN_N);


        reportFile << "\n"
				   << "DtimeG"
                   << "\t"
                   << "DtimeQ"
                   << "\n";

		writePartOfInfo(reportFile, Cell_MinReq, minValueIndex, MatrixOrData_Matrix,
                TauOrN_Tau);

		writePartOfInfo(reportFile, Cell_MinReq, minQueueValueIndex, MatrixOrData_Matrix,
                        TauOrN_N);
    }
    reportFile.close();
}

void DataManager::writePartOfInfo(std::ofstream &reportFile,
								  int cell, int index,
                                  MatrixOrData matrixOrData, TauOrN tauOrN,
                                  size_t save_count) {
    std::string str;

	switch (matrixOrData) {
        case (MatrixOrData_Matrix):
            str = std::to_string(resultMatrix[cell][index]);
            str = replacePointToComma(str, save_count);
            break;

        case (MatrixOrData_Data):
            str = std::to_string(data[cell][index]);
            str = replacePointToComma(str, save_count);
            break;

        default:
            std::cout << "Method 'writePartOfInfo' is not working correctly\n";
            exit(0);
    }

	switch (tauOrN) {
		case (TauOrN_Tau):
			reportFile << str << "\t";
			break;

		case (TauOrN_N):
			reportFile << str << "\n";
			break;

		default:
			std::cout << "Method 'writePartOfInfo' is not working correctly\n";
			exit(0);
	}
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
                writePartOfInfo(reportTable, Cell_First, index, MatrixOrData_Matrix, TauOrN_Tau);
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
                int index = findIndex(row, column);
                writePartOfInfo(reportTable, Cell_AvgReq, index, MatrixOrData_Matrix, TauOrN_Tau);
            }
        }
        reportTable << "\n";
    }
}

void DataManager::writeDowntimeInfoInTable(std::string nameOfFile) {
    std::ofstream reportTable;
    reportTable.precision(2);
    reportTable.setf(std::ios::fixed);
    reportTable.open("../../tables_26.05/eksps_2021_3_potoks/" + nameOfFile + "downtime_table.xls",
                     std::ios::trunc);
    std::string str;
    size_t neededDecimalPointsCount = 3;

    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < rowCount; ++column) {
            if (row + column + 2 < rowCount) {
                int index = findIndex(row, column);
                writePartOfInfo(reportTable, Cell_MinReq, index, MatrixOrData_Matrix, TauOrN_Tau,
                                neededDecimalPointsCount);
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
        writePartOfInfo(reportFile, Cell_AvgG, minValueIndex, MatrixOrData_Matrix,
                        TauOrN_Tau);

		// Write queue value with T1 and T3 params 
		// of the minimal value of the avg sojourn (waiting) time
        writePartOfInfo(reportFile, Cell_AvgReq, minValueIndex, MatrixOrData_Matrix,
                        TauOrN_Tau);

		// Write value of the avg sojourn (waiting) time with T1 and T3 params
		// of the minimal queue value
        writePartOfInfo(reportFile, Cell_AvgG, minQueueValueIndex, MatrixOrData_Matrix,
                        TauOrN_Tau);

		// Write minimal queue value
        writePartOfInfo(reportFile, Cell_AvgReq, minQueueValueIndex, MatrixOrData_Matrix,
                        TauOrN_Tau);

		// Write queue value for the first stream
        writePartOfInfo(reportFile, Cell_Req1, minValueIndex, MatrixOrData_Matrix, TauOrN_Tau);

		// Write queue value for the second stream
        writePartOfInfo(reportFile, Cell_Req2, minValueIndex, MatrixOrData_Matrix, TauOrN_Tau);

		// Write queue value for the third stream
        writePartOfInfo(reportFile, Cell_Req3, minValueIndex, MatrixOrData_Matrix, TauOrN_N);
	}
}

void DataManager::writeInfoAboutGammaAndQueueIntoFile() {
    std::ofstream reportFile;
    reportFile.precision(2);
    reportFile.setf(std::ios::fixed);
    reportFile.open("../../tables_26.05/eksps_2021_3_potoks/GammaAndQueueInfo.txt",
        std::ios::app);
    std::string str;

    if (minValueIndex == -1) {
        reportFile << "\n";
    } else {
		// Write avg min gamma value
        writePartOfInfo(reportFile, Cell_AvgG, minValueIndex, MatrixOrData_Matrix, TauOrN_Tau);

        // Write avg min queue value
        writePartOfInfo(reportFile, Cell_AvgReq, minQueueValueIndex, MatrixOrData_Matrix, TauOrN_Tau);

		reportFile << bestDurationsOfModes.first << "\t";
        reportFile << bestDurationsOfModes.second << "\t";

		reportFile << bestDurationsOfModesByQueueEstimate.first << "\t";
        reportFile << bestDurationsOfModesByQueueEstimate.second << "\n";
    }
    reportFile.close();
}

int DataManager::findIndex(int row, int column) {
    // It always be an integer number because it's
    // a summ of an arithmetic progression
    double index = (((2 * rowCount - (row - 1)) / 2.0) * row + column);
    return static_cast<int>(index);
}

int DataManager::calculateSizeOfVector() {
    // It always be an integer number because it's
    // a summ of an arithmetic progression
    double size = (((2 * rowCount - (rowCount - 2)) / 2.0) * (rowCount - 1) + 1);
    return static_cast<int>(size);
}

void DataManager::findCountOfSuccsessRepeats(int index) {
    successfulRepeatsCount = 0;
    for (int repeat = 0; repeat < repeatsCount; ++repeat) {
        if (data[0][index + repeat * sizeOfVector] != 0) {
            ++successfulRepeatsCount;
        }
    }
}

void DataManager::setAvgData(int index) {
    double avg = 0.0;
    double sum = 0.0;

    for (int cell = 0; cell < CONST_CELLS_COUNT; ++cell) {
        sum = 0.0;
        // Even if not all repeats were successful we can plus them (plus null)
        for (int repeat = 0; repeat < repeatsCount; ++repeat) {
            sum += data[cell][index + repeat * sizeOfVector];
        }

        // Keep it in mind that we have to divide by a number of successful repeats
        avg = sum / successfulRepeatsCount;

        resultMatrix[cell][index] = avg;
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
                    if (resultMatrix[0][index] > 0) {
                        minValueIndex = index;
                        bestDurationsOfModes = std::make_pair(row + 1, column + 1);
                        break;
                    }
                } else if ((resultMatrix[0][index] > 0) &&
                           (resultMatrix[0][index] < resultMatrix[0][minValueIndex])) {
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
                int index = findIndex(row, column);

                if (minQueueValueIndex == -1) {
                    if (resultMatrix[8][index] > 0) {
                        minQueueValueIndex = index;
                        bestDurationsOfModesByQueueEstimate = std::make_pair(row + 1, column + 1);
                        break;
                    }
                } else if ((resultMatrix[8][index] > 0) &&
                           (resultMatrix[8][index] <
                            resultMatrix[8][minQueueValueIndex])) {
                    minQueueValueIndex = index;
                    bestDurationsOfModesByQueueEstimate = std::make_pair(row + 1, column + 1);
                }
            }
        }
    }
}

std::string DataManager::replacePointToComma(std::string str, size_t save_count) {
    size_t size = str.length();
    size_t indexOfComma = size;
    for (size_t i = 0; i < size; ++i) {
        if (str[i] == '.') {
            str[i] = ',';
            indexOfComma = i;
        }
    }

    for (size_t i = size; i > indexOfComma + save_count + 1; i--) {
        str.pop_back();
    }

    return str;
}
