library( MASS )

## Function for getting data (it's discrete) from experiment with our model
get_discrete_data = function() {
  value_count <- c(45, 662, 1452, 1128, 541, 133, 36, 3)
  values <- 13:20
  data <- rep(values, value_count)
  return (data)
}

## Print histogram
print_hist = function(data) {
  min_border = floor(min(data))
  max_border = ceiling(max(data))
  data_breaks = c(min_border : max_border)
  
  hist(data, breaks = data_breaks)
}

## Print two histograms
print_two_histogram = function(first_data, second_data) {
  min_border = min( floor(min(first_data)), floor(min(second_data)) )
  max_border = max( ceiling(max(first_data)), ceiling(max(second_data)) )
  data_breaks = seq(min_border, max_border, 0.1)
  data_xlim = c(min_border, max_border)
  
  par(mfrow = c(1, 1))
  hist(first_data, breaks = data_breaks, col = "red", xlim = data_xlim)
  hist(second_data, breaks = data_breaks, col = rgb(0.5, 0.5, 0.5, 0.5), add = TRUE)
}

## Get discrete data
discrete_data = get_discrete_data()
discrete_data_length = length(discrete_data)
## Print discrete data few ways
table(discrete_data)
print_hist(discrete_data)

## Function for finding parameters for our data when we except that it's gamma distribution
find_distibution_parameters = function(data) {
  start_shape = 155
  start_rate = start_shape / 15.5
  
  parameters = fitdistr(data, "gamma", list(shape = start_shape, rate = start_rate))
  return (parameters$estimate)
}

## Find parameters for discrete data
discrete_data_parameters = find_distibution_parameters(discrete_data)
discrete_data_shape = discrete_data_parameters[1]
discrete_data_rate = discrete_data_parameters[2]

## Function for checking a simple hypothesis with ks.test
check_simple_hypothesis = function(data, data_parameteres) {
  ks.test(data, "pgamma", shape = data_parameteres[1], rate = data_parameteres[2])
}

## Check a simple hypothesis with ks.test. Reject it (because of the discrete data)
check_simple_hypothesis(discrete_data, discrete_data_parameters)

## Generate vector of gamma distribution values
genetate_gamma_distribution_data = function(data_length, data_parameteres) {
  data = rgamma(data_length, shape = data_parameteres[1], rate = data_parameteres[2])
}
gamma_data_length = discrete_data_length
gamma_data_parameters = discrete_data_parameters
gamma_data = genetate_gamma_distribution_data(gamma_data_length, gamma_data_parameters)
print_hist(gamma_data)

## Print two histograms. Not good Graphics.
## We need to round gamma distribution or to get continuous data 
print_two_histogram(discrete_data, gamma_data)

## Function for getting round data from continuous data
get_round_data = function(data) {
  return (round(data))
}
round_gamma_data = get_round_data(gamma_data)
print_two_histogram(discrete_data, round_gamma_data)
## Check that gamma data is gamma distribution, but round gamma data is not!
check_simple_hypothesis(gamma_data, gamma_data_parameters)
check_simple_hypothesis(round_gamma_data, gamma_data_parameters)

## Function for getting continuous data from discrete data.
## Generate gamma distribution and get fractional part of it,
## and then add it to our data
get_continuous_data = function(data, data_length, data_parameters) {
  gamma_data = sort(genetate_gamma_distribution_data(data_length, data_parameters))
  round_gamma_data = get_round_data(gamma_data)
  added_values = gamma_data - round_gamma_data
  data = sort(data)
  
  continuous_data = data + added_values
  return (continuous_data)
}
continious_data = get_continuous_data(discrete_data, discrete_data_length, discrete_data_parameters)
print_two_histogram(continious_data, gamma_data)
## Check that gamma data is gamma distribution, but continuous data is not! (Wrong parameters maybe)
check_simple_hypothesis(gamma_data, gamma_data_parameters)
check_simple_hypothesis(continious_data, gamma_data_parameters)
## Check that continuous round gamma data is gamma distribution.
## It's proves that our method is work.
continuous_round_gamma_data = get_continuous_data(round_gamma_data, gamma_data_length, gamma_data_parameters)
print_two_histogram(gamma_data, continuous_round_gamma_data)
check_simple_hypothesis(gamma_data, gamma_data_parameters)
check_simple_hypothesis(continuous_round_gamma_data, gamma_data_parameters)

## Now we need to find correct parameters for our data

x = seq(10, 22, by=0.1)
y = dgamma(x, shape = gamma_data_parameters[1], rate = gamma_data_parameters[2])
y = y * gamma_data_length
lines(x, y)

print_two_histogram(gamma_data, continuous_round_gamma_data)
dgamma(shape = gamma_data_parameters[1], rate = gamma_data_parameters[2])

min_border = floor(min(gamma_data))
max_border = ceiling(max(gamma_data))
data_breaks = c(min_border : max_border)

pr = table(cut(gamma_data, breaks = data_breaks))
pr = pr / sum(pr)
pr
y = c(0.000001, 0.000311, 0.008429, 0.075242, 0.247332, 0.349463, 0.228986, 0.075266, 0.013556, 0.001331, 0.000079, 0.000004)
x = c(10.5:21.5)
plot(x, y)

den = density(gamma_data)
lines(den$x, den$y, col = "blue", add = TRUE)

summary(gamma_data)
summary(continuous_round_gamma_data)

## Что, если мы снова сделаем из целых чисел действительные. И снова проверим простую гипотезу.
dobavka = runif(count, min = -0.5, max = 0.5)
not_round_x = x + dobavka
## (Работает со сто-процентным гамма-распределением).
ks.test(not_round_x, "pgamma", shape = shape_value, rate = rate_value)

## Что, если мы снова сделаем из целых чисел действительные. И снова проверим простую гипотезу. Теперь на обрезанных данных
dobavka_in_y = runif(y_count, min = -0.5, max = 0.5)
not_round_y = y + dobavka_in_y
## Не работает, хотя из 4000 значений мы выкинули всего несколько значений
ks.test(not_round_y, "pgamma", shape = shape_value, rate = rate_value)

## Теперь проверим на нашем распределении.
not_round_my_raspr_vec = my_raspr_vec + dobavka
ks.test(not_round_my_raspr_vec, "pgamma", shape = shape_value, rate = rate_value)

## Сделаеим добавку того обрезка. Гипотеза не подтверждается
new_round_my_raspr_vec = c(obrezok, my_raspr_vec)
new_count = length(new_round_my_raspr_vec)
dobavka = runif(new_count, min = -0.5, max = 0.5)
new_not_round_my_raspr_vec = new_round_my_raspr_vec + dobavka
ks.test(not_round_my_raspr_vec, "pgamma", shape = shape_value, rate = rate_value)


## Проверка на хи-квадрат. Разобъём на интервалы и сравним с вероятностями.
## Тут не так важно, что у нас только целые числа.
shape_value = 196.762749
rate_value = 12.691071
x = rgamma(count, shape = shape_value, rate = rate_value)

my_breaks = c(-Inf, 13.2, 14.4, 15.6, 16.7, 17.5, 18.7, Inf)
my_breaks
ni = table(cut(my_raspr, breaks = my_breaks))
ni
## Создадим выборку и разобьём её на данные интервалы
pr = table(cut(x, breaks = my_breaks))
pr
pr = pr / sum(pr)

a = chisq.test(x=ni, p=pr)
a$p.value

max_p_value = -1
best_shape_value = -1
best_rate_value = -1
shape_value = 192
rate_value = 12

cur_shape_value = shape_value
cur_rate_value = rate_value
for (i in 1:200) {
  cur_shape_value = cur_shape_value + 0.05
  for (j in 1:100) {
    cur_rate_value = cur_rate_value + 0.01
    x = rgamma(10000, shape = cur_shape_value, rate = cur_rate_value)
    
    pr = table(cut(x, breaks = my_breaks))
    pr = pr / sum(pr)
    
    a = chisq.test(x=ni, p=pr)
    cur_p_value = a$p.value
    if (cur_p_value > max_p_value) {
      max_p_value = cur_p_value
      best_a = a
      best_shape_value = cur_shape_value
      best_rate_value = cur_rate_value
      best_x = x
    }
  }
  cur_rate_value = rate_value
}

print(max_p_value)
print(best_shape_value)
print(best_rate_value)
print(best_a)

shape_value = best_shape_value
rate_value = best_rate_value
x = rgamma(count, shape = shape_value, rate = rate_value)

my_breaks = c(-Inf, 13.5:18.5, Inf)
my_breaks
ni = table(cut(my_raspr, breaks = my_breaks))
ni
## Создадим выборку и разобьём её на данные интервалы
pr = table(cut(x, breaks = my_breaks))
pr
pr = pr / sum(pr)

a = chisq.test(x=ni, p=pr)
a
a$p.value

## Возьмём нашу выборку, сделаем непрерывной. Подберём параметры.
par(mfrow = c(2, 1))
## Смоделированная выборка из экспериментов
ni <- c(45, 662, 1452, 1128, 541, 133, 36, 3)
count = sum(ni)
vals <- 13:20
## Создали таблицу данных
my_raspr <- rep( vals, ni )
summary(my_raspr)
table(my_raspr)
hist(my_raspr, breaks = "Sturges", xlim=c(13, 20))

dobavka = runif(count, min = -1.0, max = 1.0)
not_round_my_raspr = my_raspr + dobavka
hist(not_round_my_raspr, breaks = "Sturges", xlim=c(13, 20))
den <- density(not_round_my_raspr)
plot(den)
## При таких данных действительно похожие плотности.
par(mfrow = c(1, 1))
plot(den, xlim = c(12, 20))
x = seq(12, 20, by=0.1)
my_shape = 155
my_rate = my_shape / 15.5
y = dgamma(x, shape = my_shape, rate = my_rate)
lines(x, y)

## Предполагаем гамма распредление, ищем параметры. Проходит проверку на ks.test, но не chisq.test
x = rgamma(100000, shape = my_shape, rate = my_rate)
hist(x, breaks = "Sturges", xlim=c(13, 20))
ks.test(not_round_my_raspr, "pgamma", shape = my_shape, rate = my_rate)

my_breaks = c(-Inf, 13.2, 14.4, 15.6, 16.7, 17.5, 18.7, Inf)
my_breaks
ni = table(cut(my_raspr, breaks = my_breaks))
ni
pr = table(cut(x, breaks = my_breaks))
pr
pr = pr / sum(pr)
a = chisq.test(x=ni, p=pr)
a$statistic
a
1 - pchisq(a$statistic, df = 6)

## Проверка. Если создать две выборки гамма-распределений. Посчитать из них среднее, будет ли это выборкой с тем же распределением?
## Всё работает. Но необходима сортировка данных.
## Значит при моделировании мы можем складывать дискретные выборки и получать действительные величины.
par(mfrow = c(2, 1))
gamma_1 = rgamma(100000, shape = 155, rate = 10)
gamma_1 = sort(gamma_1)
hist(gamma_1)

gamma_2 = rgamma(100000, shape = 155, rate = 10)
gamma_2 = sort(gamma_2)
hist(gamma_2)

sum_of_gamma = (gamma_1 + gamma_2) / 2
hist(gamma_1)
hist(sum_of_gamma)
summary(gamma_1)
summary(gamma_2)
summary(sum_of_gamma)
