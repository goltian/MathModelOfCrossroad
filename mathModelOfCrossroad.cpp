#include <iostream>
#include <iomanip>
#include <omp.h>
#include <sstream>
#include <fstream>
#include "ServiceDevice.h"
#include "DataManager.h"
#include "mathModelOfCrossroad.h"

constexpr auto CONST_REPEATS_OF_ONE_EXPERIMENT = 1;

constexpr auto CONST_ORIENTATION_MODE = 3;

constexpr auto CONST_COUNT_OF_EXPERIMENTS = 1;

int main() {
    double start, end;
    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);

    // Our parameters for all experimnets
    double parametersForAll[CONST_COUNT_OF_EXPERIMENTS][9] = {
        //    g  m  g  m  l    l_p  N  pe  H

        {0.8, 4.5, 0, 1, 0.1, 0.1, 10, 30, 30},

    };

    // Cycle for all experiments
    for (int numberOfExp = 0; numberOfExp < CONST_COUNT_OF_EXPERIMENTS; numberOfExp++) {

        start = omp_get_wtime();

        int rowCount = static_cast<int>(parametersForAll[numberOfExp][8]);
        int peopleServiceModeDuration = static_cast<int>(parametersForAll[numberOfExp][7]);

        // We need to save parameters of stream for an experimnet
        std::vector<double> parametersForOne(7);
        for (int i = 0; i < 7; i++) {
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
            int firstCarsServiceModeDuration = 1;
            int secondCarsServiceModeDuration = 1;

            if (tid == 0) {
            std::cout << "Count of threads: " << omp_get_num_threads() << "\n\n";
            }

            // Cycle for filling all matrix
            do {
                int row = firstCarsServiceModeDuration;
                int column = secondCarsServiceModeDuration;

                if (tid == 0) {
                std::cout << "Exp: " << numberOfExp << "\t";
                std::cout << "Row: " << firstCarsServiceModeDuration << "\n";
                }

                // Cycle for filling one row
                do {
                    std::vector<int> modesDuration = {row,
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
                        std::vector<double> data(5);
                        data = crossroadModel.getPortionOfData();

                        // Set data into dataManager
                        matrixForOneExp.setPortionOfData((row), (column), data, tid);
                    }

                    // Increase column index
                    column++;

                    // Fill one row
                } while (row + column < rowCount);

                // Increase row index
                firstCarsServiceModeDuration++;

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
    }

    std::cout << "\nHELLO, IT`S THE END!!!!\n";
    return 0;
}

std::string getNameOfFile(const std::vector<double> parametersForOne,
                          double peopleServiceModeDuration) {
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
