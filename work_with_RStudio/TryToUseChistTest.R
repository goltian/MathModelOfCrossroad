library( MASS )

## Function for getting data (it's discrete) from experiment with our model
get_discrete_data = function() {
  value_count <- c(28, 211, 947, 2306, 3061, 2268, 902, 245, 31, 1)
  values <- -4:5
  data <- rep(values, value_count)
  return (data)
}

## Get discrete data
discrete_data = get_discrete_data()
discrete_data_length = length(discrete_data)
## Print discrete data few ways
table = table(discrete_data)
table

parameters = fitdistr(discrete_data, "normal")
parameters


names = as.numeric(names(table))
values = as.numeric(table)
names
values
freq = values / discrete_data_length
freq
sum(freq)
mean = mean(discrete_data)
sd = sd(discrete_data)

rightVals = c(-3.5:4.5, Inf)
rightVals
leftVals = c(-Inf, -3.5:4.5)
leftVals
x = c(-4:5)
z = pnorm(rightVals, mean, sd) - pnorm(leftVals, mean, sd)

z
plot(x, z, xlim=c(-5, 5), col="red", pch=15)
points(x, freq, col="blue", pch=20)

hist(dataX)
freq
z
chisq.test(freq, z)

my_mean = sum(discrete_data) / discrete_data_length
my_sd = discrete_data - my_mean
my_sd = my_var * my_var
my_sd = sum(my_sd) / (discrete_data_length - 1)
my_sd = sqrt(my_sd)
my_mean
my_sd
# They are equal
mean(discrete_data)
sd(discrete_data)
##########################################################################
## Function for getting data (it's discrete) from experiment with our model
get_discrete_data = function() {
  value_count <- c(17, 110, 560, 1343, 2028, 2223, 1762, 1143, 541, 197, 58, 14, 3, 1)
  values <- seq(13, 19.5, 0.5)
  data <- rep(values, value_count)
  return (data)
}
## Get discrete data
discrete_data = get_discrete_data()
discrete_data_length = length(discrete_data)
## Print discrete data few ways
table(discrete_data)
print_hist(discrete_data)
parameters = fitdistr(discrete_data, "gamma")
parameters


mean = mean(discrete_data)
mean
sd = sd(discrete_data)
my_shape = mean * mean / (sd * sd)
my_shape
my_rate = mean / my_shape
my_rate
