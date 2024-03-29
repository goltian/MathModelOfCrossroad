#include <iostream>
#include <iomanip>
#include <omp.h>
#include <sstream>
#include <fstream>
#include "ServiceDevice.h"
#include "DataManager.h"
#include "mathModelOfCrossroad.h"

constexpr auto CONST_REPEATS_OF_ONE_EXPERIMENT = 4;

constexpr auto CONST_ORIENTATION_MODE = 3.0;

constexpr auto CONST_COUNT_OF_EXPERIMENTS = 1;

constexpr bool CONST_OF_USING_METHOD_OF_REDUCED_BROOT_FORCE = true;

constexpr bool CONST_OF_WRITING_INFO_ABOUT_PARAMETER_N_EFFECT = false;

constexpr bool CONST_OF_WRITING_INFO_ABOUT_GAMMA_AND_QUEUE = false;

constexpr bool CONST_OF_WRITING_INFO_ABOUT_T1_T2_DURATIONS = false;

int main() {
    double start, end;
    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);

    // Our parameters for all experimnets
    double parametersForAll[CONST_COUNT_OF_EXPERIMENTS][9] = {
        //    g  m  g  m  l    l_p  N  pe  H

		{0, 1, 0, 1, 0.1, 0.1, 0, 30, 180},
    };

    // Cycle for all experiments
    for (int numberOfExp = 0; numberOfExp < CONST_COUNT_OF_EXPERIMENTS; ++numberOfExp) {

        start = omp_get_wtime();

        double rowCount = parametersForAll[numberOfExp][8];
        double peopleServiceModeDuration = parametersForAll[numberOfExp][7];

        // We need to save parameters of stream for an experimnet
        std::vector<double> parametersForOne(7);
        for (int i = 0; i < 7; ++i) {
            parametersForOne[i] = parametersForAll[numberOfExp][i];
        }

        // Create matrix using DataManager class
        DataManager matrixForOneExp(static_cast<int>(rowCount), CONST_REPEATS_OF_ONE_EXPERIMENT);
        // for (int tid = 0; tid < 4; tid++) {
        // Every repeat of experiment will be done by different threads
#pragma omp parallel num_threads(CONST_REPEATS_OF_ONE_EXPERIMENT) shared(matrixForOneExp)
        {
            // Thread get it's ID
            int tid = omp_get_thread_num();

            // Set duration of cars service modes
            double firstCarsServiceModeDuration = 1.0;
            double secondCarsServiceModeDuration = 1.0;

            if (tid == 0) {
                std::cout << "Count of threads: " << omp_get_num_threads() << "\n\n";
            }

            // Cycle for filling all matrix
            do {
                double row = firstCarsServiceModeDuration;
                double column = secondCarsServiceModeDuration;

                if (tid == 0) {
                    std::cout << "Exp: " << (numberOfExp + 1) << "\t";
                    std::cout << "Row: " << firstCarsServiceModeDuration << "\n";
                }

                // Cycle for filling one row
                do {
                    std::vector<double> modesDuration = {row,
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
                        std::vector<double> data(10);
                        data = crossroadModel.getPortionOfData();

						// Write info for visualisation
                        if ((CONST_OF_WRITING_INFO_FOR_VISUALISATION) && (tid == 0)) {
                            crossroadModel.writeInfoForVisualisationForAll();
						}

                        // Set data into dataManager
                        matrixForOneExp.setPortionOfData(static_cast<int>(row),
                                                         static_cast<int>(column), data, tid);

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
        double timeOfWork = end - start;
        std::cout << "Time: " << timeOfWork << "\n\n\n\n";

        // Get file name
        std::string nameOfFile = getNameOfFile(parametersForOne, peopleServiceModeDuration);

        // Write data into file and table
        matrixForOneExp.writeInfoInFile(peopleServiceModeDuration, parametersForOne[4], nameOfFile,
                                        timeOfWork);

        matrixForOneExp.writeInfoInTable(nameOfFile);

        matrixForOneExp.writeQueueInfoInTable(nameOfFile);

		matrixForOneExp.writeDowntimeInfoInTable(nameOfFile);

        if (CONST_OF_WRITING_INFO_ABOUT_PARAMETER_N_EFFECT) {
            matrixForOneExp.writeInfoAboutTheEffectOfParameterN(nameOfFile);
        }

		if (CONST_OF_WRITING_INFO_ABOUT_GAMMA_AND_QUEUE) {
			matrixForOneExp.writeInfoAboutGammaAndQueueIntoFile();
		}

		if (CONST_OF_WRITING_INFO_ABOUT_T1_T2_DURATIONS) {
            matrixForOneExp.writeInfoAboutT1AndT2Durations();
		}
    }

    std::cout << "\nHELLO, IT'S THE END!!!!\n";
    return 0;
}

std::string getNameOfFile(const std::vector<double> parametersForOne,
                          double peopleServiceModeDuration) {
    std::ostringstream stream;

    stream << std::fixed;
    //    g  m  g  m  l    l_p  N  pe  H
    stream << std::setprecision(2);
    stream << "l=";
    stream << parametersForOne[4];
    stream << "__";

	stream << std::setprecision(2);
    stream << "l_p=";
    stream << parametersForOne[5];
    stream << "__";

    stream << std::setprecision(2);
    stream << "g=";
    stream << parametersForOne[0];
    stream << "__";

    double r = (1.0 - parametersForOne[0]) * (parametersForOne[1] - 1.0);
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

void computeNextIndexes(bool crossroadIsWorking, double secondCarsServiceModeDuration,
                        double firstCarsServiceModeDuration, double rowCount, double &row,
                        double &column) {
    if (CONST_OF_USING_METHOD_OF_REDUCED_BROOT_FORCE) {
        if (crossroadIsWorking) {
            if ((row + column + 1 == rowCount) && (column > row)) {
                column = secondCarsServiceModeDuration;
                row = firstCarsServiceModeDuration + 1;
            } else {
                if (column >= row)
                    // Go down while cross road is working
                    ++column;
                else
                    // Go right while cross road is working
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
