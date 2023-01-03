library( MASS )

par(mfrow = c(2, 1))
## Смоделированная выборка из экспериментов
ni <- c(45, 662, 1452, 1128, 541, 133, 36, 3)
count = sum(ni)
vals <- 13:20
## Создали таблицу данных
my_raspr <- rep( vals, ni )
table(my_raspr)
hist(my_raspr, breaks = "Sturges", xlim=c(13, 20))
## Предполагаем гамма распредление, ищем параметры
fitdistr( my_raspr, "gamma", list(shape = 195, rate = 12.5)) -> ins.pois
ins.pois
shape_value = 196.762749
rate_value = 12.691071

## Проверяем простую гипотезу. Не даёт результата, поскольку у нас только дискретные значения
my_raspr_vec <- as.vector(my_raspr)
ks.test(my_raspr_vec, "pgamma", shape = shape_value, rate = rate_value)

## Генерируем гамма распределение с найденными параметрыми
x = rgamma(count, shape = shape_value, rate = rate_value)
round_x = round(x)

y = round_x[round_x >= 13]
obrezok = round_x[round_x < 13]
obrezok
y = y[y <= 20]
y_count = length(y)
##table(round_x)
table(my_raspr)
table(y)
## Проводим визуальное сравнение
hist(round_x, breaks = "Sturges")
hist(my_raspr, breaks = "Sturges", xlim=c(13, 20))
hist(y, breaks = "Sturges", xlim=c(13, 20))

## Проверяем простую гипотезу. Гипотеза подтверждается
ks.test(x, "pgamma", shape = shape_value, rate = rate_value)

## Проверяем простую гипотезу. Гипотеза не подтверждается, снова из-за округления, хотя мы точно знаем, что это гамма распределение
y_vec <- as.vector(round_x)
ks.test(y_vec, "pgamma", shape = shape_value, rate = rate_value)

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


## Предполагаем отр бином распредление, ищем параметры
fitdistr( my_raspr, "negative binomial") -> ins.pois
ins.pois
size_value = 1.906463e+03
size_value
mu_value = 1.550397e+01
mu_value
## Генерируем отр бином распределение с найденными параметрыми. Это не оно
x = rnbinom(count, size = size_value, mu = mu_value)
y = x[x >= 13]
y = y[y <= 20]
table(x)
table(y)
hist(x, breaks = "Sturges")
hist(y, breaks = "Sturges", xlim=c(13, 20))
