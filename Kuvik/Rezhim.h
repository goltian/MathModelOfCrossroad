#pragma once
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <random>
#include <conio.h>

class rezhim {
private:
    double liam;
    double r;
    double g;
    std::mt19937 gen1;
    std::uniform_real_distribution<> urd1;
    double min_delta_mashin;
    int N; // количество заявок, поступивших за очередной интервал
    double time_of_tact; // массив времён каждого такта
    std::vector <double> T; // массив значений времени, в которые будут поступать все заявки
    std::vector <int> Kolvo_Bistrix; // количество быстрых машин в каждой пачке
    std::vector <double> T_bistrix_mashin; // время прихода быстрых машин
    int SUM_T; // суммарное время всех интервалов
    int sum_n; // общее число заявок за все интервалы
    int k; // счётчик количества интервалов
    int sum_bistrix_mashin; // количество всех быстрых машин
    int bunker_nakop;
    int num_nasish;

public:
    rezhim(double, double, double, double, std::mt19937 &, 
        std::uniform_real_distribution<> &, double);

    void set_params(double, double, double, double, std::mt19937 &, std::uniform_real_distribution<> &, double);

    void set_rand_tact(double);

    int get_bunker_nak();

    int get_num_nas();

    void print();

    std::vector <double> get_times_slow();

    std::vector <int> get_num_fast();

    std::vector <double> get_times_fast();

    void potok_in_work(); // подсчёт машин за время rand_t для открытого направления

    void potok_in_waiting();

    void potok_nasish();
};
