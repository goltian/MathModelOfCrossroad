#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <random>
#include <conio.h>
#include "Get_Puas_and_Bart.h"
#include "rezhim.h"

void Puas(double liam, int t, double r, double g, std::mt19937 &gen1,
    std::uniform_real_distribution<> &urd1) // теоретеческое значение лямбды и время одного интервала
{
    std::vector <double> T; // массив значений времени, в которые будут поступать все заявки
    int SUM_T = 0; // суммарное время всех интервалов
    double liam_st = 0; // статистичсеское значение лямбда
    int N = 0; // количество заявок, поступивших за очередной интервал
    int sum_n = 0; // общее число заявок за все интервалы
    int k = 0; // счётчик количества интервалов
    //std::vector <int> T_poslednie; // последние заявки
    double mat_ozid; // среднее число быстрых машин в пачке
    double liam_bart; // среднее число медленных машин в очередном интервале
    std::vector <int> Kolvo_Medlennix; // сколько медленных машин было в каждом интервале
    std::vector <int> Kolvo_Bistrix; // количество быстрых машин в каждой пачке
    double p; // значение случайной величины
    int q; // сколько было заявок за всё время до текущего интервала
    std::vector <double> Delta_Pachki; // расстояние (время) между пачками
    double delta_mashin; // расстояние (время) между машинами в пачке
    std::vector <double> T_bistrix_mashin; // время прихода быстрых машин
    int sum_bistrix_mashin = 0; // количество всех быстрых машин
    double r_st = 0; // вероятность поступления в пачке больше одной машины (статистическое значение)
    int it_r_st_1 = 0; // итератор для пачек > 1 машины
    int it_r_st_2 = 0; // итератор для пачек = 1 машине
    double mat_ozid_st = 0; // среднее число быстрых машин в пачке (статистическое значение)
    double liam_bart_st; // число медленных машин (статистическое значение)

    mat_ozid = 1 + r / (1 - g); // считаем среднее число быстрых машин в пачке (теоретическое)
    liam_bart = liam / mat_ozid; // считаем среднее число медленных машин в очередном интервале (теоретическое)

    std::cout << "модуль разницы между значениями лямбд должен быть меньше, чем: " << 0.1 * liam << "\n";
    std::cout << "модуль разницы между значениями лямбд Бартледа должен быть меньше, чем: " << 0.1 * liam_bart << "\n";
    std::cout << "модуль разницы между значениями r должен быть меньше, чем: " << 0.1 * r << "\n";

    do
    {
        do
        {
            do // идём в цикле, пока разность двух лямбд по модулю не будет меньше теоретической лямбды, умноженной на 0.1
            {
                N = Get_Puasson(liam_bart, t, gen1, urd1);

                if ((N == 0) && (k == 0)) // начинаем вычисления только с того интервала, за который поступила хотя бы одна заявка
                    continue;

                Kolvo_Medlennix.push_back(N);

                T.resize(sum_n + N); // меняем размер массива заявок для добавления новых
                Kolvo_Bistrix.resize(sum_n + N); // меняем размер массива количества быстрых машин в пачках для добавления новых
                Delta_Pachki.resize(sum_n + N); // меняем размер массива разницы во времени между пачками

                q = sum_n; // сколько было заявок за всё время до текущего интервала
                while (q < sum_n + N) // цикл, в котором мы заполняем массив времён, в которые пришли заявки за очередной интервал
                {
                    p = urd1(gen1); // очередное случайное значение от 0 до 1
                    T[q] = (double)t*p + k * t; // время, в которое пришла очередная заявка (медленные машины)
                    q++;
                }
                //std::cout << "\n";
                //std::cout << liam_st; // вывод этого значения на экран
                std::sort(T.begin(), T.end()); // Упорядочивание времён для очередного интервала

                for (int i = sum_n; i < sum_n + N; i++) // идём в цикле, высчитывая для каждой пачки число быстрых машин
                {
                    Kolvo_Bistrix[i] = Get_Bartlet(r, g, gen1, urd1);
                    sum_bistrix_mashin += Kolvo_Bistrix[i];
                    if (Kolvo_Bistrix[i] > 0) // увеличиваем it_r_st_1 в случае, если в пачке больше одной машины
                        it_r_st_1++;
                    if (Kolvo_Bistrix[i] == 0) // увеличиваем it_r_st_2 в случае, если в пачке одна машина
                        it_r_st_2++;
                }
                int smesh = 0;
                T_bistrix_mashin.resize(sum_bistrix_mashin);
                for (int i = sum_n; i < sum_n + N; i++)// идём в цикле, высчитывая промежутки между пачками,
                    // промежутки между машинами для каждой пачки и время прихода быстрых машин
                {
                    if (i < sum_n + N - 1)
                        Delta_Pachki[i] = (double)(T[i + 1] - T[i]) / 2;
                    else
                        Delta_Pachki[i] = (t * (k + 1) - T[i]) / 2;
                    if (i > 0)
                        smesh += Kolvo_Bistrix[i - 1];
                    delta_mashin = (double)Delta_Pachki[i] / Kolvo_Bistrix[i];
                    std::cout << "\nвремя прихода быстрых машин: ";
                    for (int j = 0; j < Kolvo_Bistrix[i]; j++)
                    {
                        T_bistrix_mashin[j + smesh] = T[i] + (j + 1) * delta_mashin;
                        std::cout << T_bistrix_mashin[j + smesh] << " ";
                    }
                    std::cout << "\n";
                }

                //std::cout << "delta " << i << "-ой пачке = " << Delta[i] << "\n";
            //std::cout << "максимальное количество быстрых машин в " << k << "-ом интервале = " << max_kolvo_bist << "\n";



                k++; // увеличение счётчика интервалов
                std::cout << "\nинтервал номер " << k;
                std::cout << ". поступило заявок: " << N << "\n";
                for (int i = sum_n; i < sum_n + N; i++) // вывод заявок из текущего интервала
                {
                    std::cout << T[i] << " ";
                    std::cout << "количество быстрых машин в " << i << "-ой пачке = " << Kolvo_Bistrix[i] << "\n";
                }

                SUM_T += t; // пересчитываем суммарное время интервалов
                sum_n += N; // пересчитываем количество заявок (медленных машин) за всё время с учётом текущего интервала
                r_st = (double)it_r_st_1 / (it_r_st_1 + it_r_st_2);
                //mat_ozid_st = (sum_n + sum_bistrix_mashin) / sum_n; // пересчитываем среднее количество машин в пачке
                liam_bart_st = (double)sum_n / SUM_T; // вычисление статистического значения лямбды Бартледа
                liam_st = (double)(sum_n + sum_bistrix_mashin) / SUM_T; // вычисление статистического значения лямбды

                std::cout << "\n";
                std::cout << "модуль разницы между значениями лямбд равен: " << abs(liam - liam_st) << "\n";
                std::cout << " " << (liam_st) << " " << SUM_T << "\n";
            } while (abs(liam - liam_st) >= (0.1 * liam));
            //std::cout << "\n";
            std::cout << "модуль разницы между значениями лямбд Бартледа равен: " << abs(liam_bart - liam_bart_st) << "\n";
            std::cout << " " << (liam_bart_st) << "\n";
        } while (abs(liam_bart - liam_bart_st) >= (0.1 * liam_bart));
        //std::cout << "\n";
        std::cout << "модуль разницы между значениями r равен: " << abs(r - r_st) << "\n";
        std::cout << " " << (r_st) << "\n";
    } while (abs(r - r_st) >= (0.1 * r));
    //std::cout <<"\n" << abs(a - lim_ st) << " " << (0.1 * a) << " "<< SUM_T << "\n";
    std::cout << SUM_T; // выводим суммарное время
    //T_poslednie.resize(N);
    //int u = sum_n - N;
    //for (int i = 0; i < N; i++)
    //{
    //    T_poslednie[i] = T[u] - SUM_T + t; // заявки последнего интервала
    //    u++;
    //}
    //return T_poslednie;
    return;
}

int main()
{
    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);

    std::mt19937 gen(time(0));
    std::uniform_real_distribution<> urd(0, 1);
    //std::uniform_int_distribution <int> urd(0, 1000);
    std::vector<int> vremia (5, 0);
    setlocale(LC_ALL, "Russian");
    //Get_Puasson(0.2, 50, gen, urd);
    //Get_Bartlet(0.7, 0.8, gen, urd);
    //Puas(0.5, 50, 0.7, 0.8, gen, urd);
    double rand_t = urd(gen) * 50;
    rezhim potok_1(0.5, 0.7, 0.8, rand_t, gen, urd, 0.5);
    rezhim potok_2(0.5, 0.7, 0.8, rand_t, gen, urd, 0.5);
    rezhim potok_3(0.5, 0.7, 0.8, rand_t, gen, urd, 0.5);

    int i = 0;
    double p;
    int kolvo = 0;
    while (i < 3) {
        i++;
        p = urd(gen) * 10;
        potok_1.set_rand_tact(p);
        potok_2.set_rand_tact(p);
        potok_3.set_rand_tact(p);
        std::cout << "размер такта = " << p << "\n";
        potok_1.potok_in_work();
        potok_1.potok_nasish();
        potok_2.potok_in_waiting();
        potok_3.potok_in_waiting();
        potok_1.print();

        std::cout << "\n\n\n\n\n";
        std::cout << potok_1.get_num_nas() << "\n";
    }

    std::cout << potok_2.get_bunker_nak() << "\n";

    _getch();
    return 0;
}