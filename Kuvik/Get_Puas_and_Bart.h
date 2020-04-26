#pragma once
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <random>
#include <conio.h>

int Get_Puasson(double liam, int t, std::mt19937 &gen1,
    std::uniform_real_distribution<> &urd1);

int Get_Bartlet(double r, double g, std::mt19937 &gen1,
    std::uniform_real_distribution<> &urd1); // теоретеческое значение лямбды и время одного интервала