#include <iostream>
#include <iomanip>
#include <omp.h>
#include <sstream>
#include <fstream>
#include "ServiceDevice.h"
#include "DataManager.h"
#include "mathModelOfCrossroad.h"

constexpr auto CONST_REPEATS_OF_ONE_EXPERIMENT = 1;

constexpr auto CONST_ORIENTATION_MODE = 3.0F;

constexpr auto CONST_COUNT_OF_EXPERIMENTS = 1;

constexpr bool CONST_OF_USING_METHOD_OF_REDUCED_BROOT_FORCE = false;

int main() {
    float start, end;
    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);

    // Our parameters for all experimnets
    float parametersForAll[CONST_COUNT_OF_EXPERIMENTS][9] = {
        //    g  m  g  m  l    l_p  N  pe  H

        {0.91, 2, 0, 1, 0.2, 0.1, 0, 30, 90},

    };

    // Cycle for all experiments
    for (int numberOfExp = 0; numberOfExp < CONST_COUNT_OF_EXPERIMENTS; ++numberOfExp) {

        start = omp_get_wtime();

        float rowCount = parametersForAll[numberOfExp][8];
        float peopleServiceModeDuration = parametersForAll[numberOfExp][7];

        // We need to save parameters of stream for an experimnet
        std::vector<float> parametersForOne(7);
        for (int i = 0; i < 7; ++i) {
            parametersForOne[i] = parametersForAll[numberOfExp][i];
        }

        // Create matrix using DataManager class
        DataManager matrixForOneExp(rowCount, CONST_REPEATS_OF_ONE_EXPERIMENT);
        // for (int tid = 0; tid < 4; tid++) {
        // Every repeat of experiment will be done by different threads
#pragma omp parallel num_threads(CONST_REPEATS_OF_ONE_EXPERIMENT) shared(matrixForOneExp)
        {
            // Thread get it`s ID
            int tid = omp_get_thread_num();

            // Set duration of cars service modes
            float firstCarsServiceModeDuration = 1.0F;
            float secondCarsServiceModeDuration = 1.0F;

            if (tid == 0) {
                std::cout << "Count of threads: " << omp_get_num_threads() << "\n\n";
            }

            // Cycle for filling all matrix
            do {
                float row = firstCarsServiceModeDuration;
                float column = secondCarsServiceModeDuration;

                if (tid == 0) {
                    std::cout << "Exp: " << numberOfExp << "\t";
                    std::cout << "Row: " << firstCarsServiceModeDuration << "\n";
                }

                // Cycle for filling one row
                do {
                    std::vector<float> modesDuration = {row,
                                                        CONST_ORIENTATION_MODE,
                                                        column,
                                                        CONST_ORIENTATION_MODE,
                                                        peopleServiceModeDuration,
                                                        CONST_ORIENTATION_MODE,
                                                        peopleServiceModeDuration,
                                                        CONST_ORIENTATION_MODE};

                    // Create crossroad model with the specified parameters
                    ServiceDevice crossroadModel(parametersForOne, modesDuration);

                    // If our model is working then write information into DataManager
                    if (crossroadModel.isCrossroadModelWorksStably()) {

                        // Get data from model
                        std::vector<float> data(5);
                        data = crossroadModel.getPortionOfData();

                        // Set data into dataManager
                        matrixForOneExp.setPortionOfData((row), (column), data, tid);

                        bool crossroadIsWorking = true;
                        computeNextIndexes(crossroadIsWorking, secondCarsServiceModeDuration,
                                           firstCarsServiceModeDuration, rowCount, row, column);

                    } else {
                        bool crossroadIsWorking = false;
                        computeNextIndexes(crossroadIsWorking, secondCarsServiceModeDuration,
                                           firstCarsServiceModeDuration, rowCount, row, column);
                    }

                    // Fill one row
                } while (row + column < rowCount);

                // Increase row index
                if (CONST_OF_USING_METHOD_OF_REDUCED_BROOT_FORCE) {
                    ++firstCarsServiceModeDuration;
                    ++secondCarsServiceModeDuration;
                } else {
                    ++firstCarsServiceModeDuration;
                }

                // Fill all matrix
            } while (firstCarsServiceModeDuration + secondCarsServiceModeDuration < rowCount);
            // Synchronization
#pragma omp barrier
        }
        // }

        end = omp_get_wtime();
        float timeOfWork = end - start;
        std::cout << "Time: " << timeOfWork << "\n\n\n\n";

        // Get file name
        std::string nameOfFile = getNameOfFile(parametersForOne, peopleServiceModeDuration);

        // Write data into file and table
        matrixForOneExp.writeInfoInFile(peopleServiceModeDuration, parametersForOne[4], nameOfFile,
                                        timeOfWork);

        matrixForOneExp.writeInfoInTable(nameOfFile);
    }

    std::cout << "\nHELLO, IT`S THE END!!!!\n";
    return 0;
}

std::string getNameOfFile(const std::vector<float> parametersForOne,
                          float peopleServiceModeDuration) {
    std::ostringstream stream;

    stream << std::fixed;
    //    g  m  g  m  l    l_p  N  pe  H
    stream << std::setprecision(1);
    stream << "l=";
    stream << parametersForOne[4];
    stream << "__";

    stream << "l_p=";
    stream << parametersForOne[5];
    stream << "__";

    stream << std::setprecision(2);
    stream << "g=";
    stream << parametersForOne[0];
    stream << "__";

    float r = (1.0F - parametersForOne[0]) * (parametersForOne[1] - 1.0F);
    stream << "r=";
    stream << r;
    stream << "__";

    stream << "m_o=";
    stream << parametersForOne[1];
    stream << "__";

    stream << std::setprecision(0);
    stream << "N=";
    stream << parametersForOne[6];
    stream << "__";

    stream << "Peop=";
    stream << peopleServiceModeDuration;
    stream << "__";

    return stream.str();
}

void computeNextIndexes(bool crossroadIsWorking, float secondCarsServiceModeDuration,
                        float firstCarsServiceModeDuration, float rowCount, float &row,
                        float &column) {
    if (CONST_OF_USING_METHOD_OF_REDUCED_BROOT_FORCE) {
		if (crossroadIsWorking) {
			if ((row + column + 1 == rowCount) && (column > row)) {
				column = secondCarsServiceModeDuration;
				row = firstCarsServiceModeDuration + 1;
			} else {
				if (column >= row)
					// идём вниз, пока не получим аварийный режим
					++column;
				else
					// идём вправо, пока не получим аварийный режим
					++row;
			}
		} else {
			if (column == row) {
				++column;
			}

			if (column > row) {
				column = secondCarsServiceModeDuration;
				row = firstCarsServiceModeDuration + 1;
			} else {
				row = rowCount;
			}
		}
    } else {
        // Increase column index
        ++column;
	}
}