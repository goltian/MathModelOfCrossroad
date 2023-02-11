library( MASS )

## Function for getting data (it's discrete) from experiment with our model
get_data = function() {
  frequencies <- c(28, 211, 947, 2306, 3061, 2268, 902, 245, 31)
  values <- -4:4
  data <- rep(values, frequencies)
  return (data)
}

## Get data
data = get_data()
data_length = length(data)

table = table(data)
values = as.numeric(names(table))
frequencies = as.numeric(table)
values
frequencies

## Compute mean and sd
parameters = fitdistr(data, "normal")
mean = mean(data)
sd = sd(data)
mean
sd

## Get probabilities for the intervals
rightVals = c(-3.5:3.5, Inf)
leftVals = c(-Inf, -3.5:3.5)
x = c(-4:4)
probs = pnorm(rightVals, mean, sd) - pnorm(leftVals, mean, sd)

## Chisq test does not work with sample mean and sd
chisq.test(frequencies, p=probs)

## Optimize p value and find best sd (mean = 0)
optimize_data_sd = function(cur_data_sd) {
  cur_data_mean = 0
  probs = ( pnorm(rightVals, cur_data_mean, cur_data_sd) - 
    pnorm(leftVals, cur_data_mean, cur_data_sd) )
  cur_res = chisq.test(frequencies, p=probs)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_sd, 
                           interval = c(1, 2), maximum = TRUE, tol = 0.0001)
best_sd = optimize_result$maximum
best_sd

probs = pnorm(rightVals, 0, best_sd) - pnorm(leftVals, 0, best_sd)
plot(x, probs, xlim=c(-5, 5), col="green", pch=4)
points(x, frequencies / data_length, col="blue", pch=20)

## With best sd and mean = 0 chisq test works
chisq.test(frequencies, p=probs)

##########################################################################
## Function for getting data (it's discrete) from experiment with our model
get_discrete_data = function() {
  value_count <- c(17, 110, 560, 1343, 2028, 2223, 1762, 1143, 541, 197, 58, 18)
  values <- seq(13, 18.5, 0.5)
  data <- rep(values, value_count)
  return (data)
}
## Get discrete data
data = get_discrete_data()
discrete_data_length = length(data)
## Print discrete data few ways
table = table(data)
print_hist(data)
parameters = fitdistr(data, "gamma")
parameters = parameters$estimate
shape = parameters[1]
shape
rate = parameters[2]
rate
names = as.numeric(names(table))
values = as.numeric(table)
names
values
mean(data)

rightVals = c(seq(13.25, 18.25, 0.5), Inf)
rightVals
length(rightVals)
leftVals = c(-Inf, seq(13.25, 18.25, 0.5))
leftVals
x = seq(13, 18.5, 0.5)
length(x)
length(z)
z = pgamma(rightVals, shape, rate) - pgamma(leftVals, shape, rate)

z
plot(x, z, col="green", pch=4)
freq = values / discrete_data_length
points(x, freq, col="blue", pch=20)

freq
values
z
chisq.test(values, p=z)

optimize_data_sd = function(cur_data_shape) {
  cur_data_rate = cur_data_shape / 15.4969
  z = pgamma(rightVals, cur_data_shape, cur_data_rate) - pgamma(leftVals, cur_data_shape, cur_data_rate)
  cur_res = chisq.test(values, p=z)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_sd, interval = c(300, 320), maximum = TRUE, tol = 0.0001)
best_shape = optimize_result$maximum
best_shape
best_rate = best_shape / 15.4969

z = pgamma(rightVals, best_shape, best_rate) - pgamma(leftVals, best_shape, best_rate)
z
plot(x, z, col="green", pch=4)
points(x, freq, col="blue", pch=20)

freq
values
z
chisq.test(values, p=z)
ks.test(freq, z)
