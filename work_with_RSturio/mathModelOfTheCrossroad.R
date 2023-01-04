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

den <- density(my_raspr)
plot(den)

par(mfrow = c(1, 1))
plot(den, xlim = c(12, 20))
x = seq(12, 20, by=0.1)
my_shape = 155
my_rate = my_shape / 15.5
y = dgamma(x, shape = my_shape, rate = my_rate)
lines(x, y)

## Предполагаем гамма распредление, ищем параметры
x = rgamma(count, shape = my_shape, rate = my_rate)
hist(x, breaks = "Sturges", xlim=c(13, 20))

y_vec <- as.vector(round_x)
ks.test(not_round_my_raspr, "pgamma", shape = my_shape, rate = my_rate)
