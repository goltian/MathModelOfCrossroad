#include "rezhim.h"
#include "Get_Puas_and_Bart.h"

rezhim::rezhim(double _liam, double _r, double _g, double _time_of_tact,
    std::mt19937 & _gen1, std::uniform_real_distribution<> & _urd1, double _min_delta_mashin) {
    set_params(_liam, _r, _g, _time_of_tact, _gen1, _urd1, _min_delta_mashin);
    SUM_T = 0; // суммарное время всех интервалов
    sum_n = 0; // общее число заявок за все интервалы
    k = 0; // счётчик количества интервалов
    sum_bistrix_mashin = 0; // количество всех быстрых машин
    bunker_nakop = 0; // количество заявок, которые ожидают включения их режима
    num_nasish = 0; // макс. кол-во заявок, которых бы можно было обслужить
}

void rezhim::set_params(double _liam, double _r, double _g, double _time_of_tact,
    std::mt19937 & _gen1, std::uniform_real_distribution<> & _urd1, double _min_delta_mashin) {
    liam = _liam;
    r = _r;
    g = _g;
    time_of_tact = _time_of_tact;
    gen1 = _gen1;
    urd1 = _urd1;
    min_delta_mashin = _min_delta_mashin;
}

void rezhim::set_rand_tact(double _time_of_tact) {
    time_of_tact = _time_of_tact;
}

int rezhim::get_bunker_nak() {
    return bunker_nakop;
}

int rezhim::get_num_nas() {
    return num_nasish;
}

void rezhim::print()
{
    int smesh = 0;
    std::cout << "медлен" << "      быстрые\n";
    for (int i = sum_n - N; i < sum_n; i++)// идём в цикле, высчитывая промежутки между пачками,
    // промежутки между машинами для каждой пачки и время прихода быстрых машин
    {
        if (i > 0)
            smesh += Kolvo_Bistrix[i - 1];
        std::cout << T[i] << "      ";
        for (int j = 0; j < Kolvo_Bistrix[i]; j++)
        {
            std::cout << T_bistrix_mashin[j + smesh] << " ";
        }
        std::cout << "\n";
    }
}

std::vector <double> rezhim::get_times_slow() {
    return T;
}

std::vector <int> rezhim::get_num_fast() {
    return Kolvo_Bistrix;
}

std::vector <double> rezhim::get_times_fast() {
    return T_bistrix_mashin;
}

void rezhim::potok_in_work() { // подсчёт машин за время rand_t
    std::vector <double> Delta_Pachki; // расстояние (время) между пачками
    double delta_mashin; // расстояние (время) между машинами в пачке
    double liam_bart;
    double mat_ozid;
    double Kolvo_Medlennix;
    double rand_t;
    double p; // значение случайной величины
    int q;
    rand_t = time_of_tact;
    mat_ozid = 1 + r / (1 - g); // считаем среднее число быстрых машин в пачке (теоретическое)

    liam_bart = liam / mat_ozid; // считаем среднее число медленных машин в очередном интервале (теоретическое)

    N = Get_Puasson(liam_bart, rand_t, gen1, urd1);

    T.resize(sum_n + N); // меняем размер массива заявок для добавления новых
    Kolvo_Bistrix.resize(sum_n + N); // меняем размер массива количества быстрых машин в пачках для добавления новых
    Delta_Pachki.resize(sum_n + N); // меняем размер массива разницы во времени между пачками

    q = sum_n; // сколько было заявок за всё время до текущего интервала
    while (q < sum_n + N) // цикл, в котором мы заполняем массив времён, в которые пришли заявки за очередной интервал
    {
        p = urd1(gen1); // очередное случайное значение от 0 до 1
        T[q] = rand_t*p + SUM_T; // время, в которое пришла очередная заявка (медленные машины)
        q++;
    }
    std::sort(T.begin(), T.end()); // Упорядочивание времён для очередного интервала

    for (int i = sum_n; i < sum_n + N; i++) // идём в цикле, высчитывая для каждой пачки число быстрых машин
    {
        Kolvo_Bistrix[i] = Get_Bartlet(r, g, gen1, urd1);
        sum_bistrix_mashin += Kolvo_Bistrix[i];
    }
    int smesh = 0;
    T_bistrix_mashin.resize(sum_bistrix_mashin);
    for (int i = sum_n; i < sum_n + N; i++)// идём в цикле, высчитывая промежутки между пачками,
        // промежутки между машинами для каждой пачки и время прихода быстрых машин
    {
        if (i < sum_n + N - 1)
            Delta_Pachki[i] = (double)(T[i + 1] - T[i]) / 2;
        else
            Delta_Pachki[i] = (rand_t * (k + 1) - T[i]) / 2;
        if (i > 0)
            smesh += Kolvo_Bistrix[i - 1];
        delta_mashin = (double)Delta_Pachki[i] / Kolvo_Bistrix[i];
        if (min_delta_mashin > delta_mashin)
            min_delta_mashin = delta_mashin;
        for (int j = 0; j < Kolvo_Bistrix[i]; j++)
        {
            T_bistrix_mashin[j + smesh] = T[i] + (j + 1) * delta_mashin;
        }
    }

    k++; 
    SUM_T += rand_t; // пересчитываем суммарное время интервалов
    sum_n += N; // пересчитываем количество заявок (медленных машин) за всё время с учётом текущего интервала
}

void rezhim::potok_in_waiting()
{
    double liam_bart;
    double mat_ozid;
    double num_medl;
    double num_bist = 0;
    double rand_t;

    rand_t = time_of_tact;
    mat_ozid = 1 + r / (1 - g); // считаем среднее число быстрых машин в пачке (теоретическое)

    liam_bart = liam / mat_ozid; // считаем среднее число медленных машин в очередном интервале (теоретическое)

    num_medl = Get_Puasson(liam_bart, rand_t, gen1, urd1);

    for (int i = 0; i < num_medl; i++) // идём в цикле, высчитывая для каждой пачки число быстрых машин
    {
        num_bist += Get_Bartlet(r, g, gen1, urd1);
    }

    bunker_nakop += (num_medl + num_medl);
}

void rezhim::potok_nasish()
{
    double num_medl;
    double num_bist = 0;
    double rand_t;
    double delta_mashin; // расстояние (время) между машинами в пачке

    delta_mashin = min_delta_mashin;
    rand_t = time_of_tact;
    
    num_nasish = (int)(rand_t / delta_mashin);
}
