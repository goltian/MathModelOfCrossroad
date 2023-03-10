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

## Optimize p value and find best sd (mean = prev mean)
optimize_data_sd = function(cur_data_sd) {
  cur_data_mean = mean
  probs = ( pnorm(rightVals, cur_data_mean, cur_data_sd) - 
    pnorm(leftVals, cur_data_mean, cur_data_sd) )
  cur_res = chisq.test(frequencies, p=probs)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_sd, 
                           interval = c(1, 2), maximum = TRUE, tol = 0.0001)
best_sd = optimize_result$maximum
best_sd

probs = pnorm(rightVals, mean, best_sd) - pnorm(leftVals, mean, best_sd)
plot(xlim = c(-4, 4), ylim = c(0, 0.4), x = -1, y = -1)
points(x, probs, col="red", pch=20, lwd = 10)
points(x, frequencies / data_length, col="blue", pch=20, lwd = 2)

## With best sd and mean = prev mean chisq test works
chisq.test(frequencies, p=probs)

############################################################################
## Same tests for 50 000 exps

## Function for getting data (it's discrete) from experiment with our model
get_data = function() {
  frequencies <- c(169, 1052, 4644, 11367, 15468, 11578, 4502, 1048, 172)
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

## Optimize p value and find best sd (mean = prev mean)
optimize_data_sd = function(cur_data_sd) {
  cur_data_mean = mean
  probs = ( pnorm(rightVals, cur_data_mean, cur_data_sd) - 
              pnorm(leftVals, cur_data_mean, cur_data_sd) )
  cur_res = chisq.test(frequencies, p=probs)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_sd, 
                           interval = c(1, 2), maximum = TRUE, tol = 0.0001)
best_sd = optimize_result$maximum
best_sd

probs = pnorm(rightVals, mean, best_sd) - pnorm(leftVals, mean, best_sd)
plot(xlim = c(-4, 4), ylim = c(0, 0.4), x = -1, y = -1)
points(x, probs, col="red", pch=20, lwd = 10)
points(x, frequencies / data_length, col="blue", pch=20, lwd = 2)

## With best sd and mean = prev mean chisq test works
chisq.test(frequencies, p=probs)


############################################################################
## Is it the same dist for 10 000 and 50 000 exps?
probs = c(169, 1052, 4644, 11367, 15468, 11578, 4502, 1048, 172) / 50000

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
frequencies

## Chisq test does not work with sample mean and sd
chisq.test(frequencies, p=probs)
x = c(-4:4)
plot(xlim = c(-4, 4), ylim = c(0, 0.4), x = -1, y = -1)
points(x, probs, col="red", pch=20, lwd = 10)
points(x, frequencies / data_length, col="blue", pch=20, lwd = 2)

############################################################################

## Same tests for Bartlett P1 and P2 flows
## Function for getting data (it's discrete) from experiment with our model
get_data = function() {
  frequencies <- c(15, 34, 124, 315, 624, 1169, 1710, 1992, 
                   1644, 1226, 654, 317, 121, 38, 16)
  values <- -7:7
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
mean = mean(data)
sd = sd(data)
mean
sd

## Get probabilities for the intervals
rightVals = c(-6.5:6.5, Inf)
leftVals = c(-Inf, -6.5:6.5)
x = c(-7:7)
probs = pnorm(rightVals, mean, sd) - pnorm(leftVals, mean, sd)

## Chisq test does not work with sample mean and sd
chisq.test(frequencies, p=probs)

## Optimize p value and find best sd (mean = prev mean)
optimize_data_sd = function(cur_data_sd) {
  cur_data_mean = mean
  probs = ( pnorm(rightVals, cur_data_mean, cur_data_sd) - 
              pnorm(leftVals, cur_data_mean, cur_data_sd) )
  cur_res = chisq.test(frequencies, p=probs)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_sd, 
                           interval = c(2, 2.5), maximum = TRUE, tol = 0.0001)
best_sd = optimize_result$maximum
best_sd

probs = pnorm(rightVals, mean, best_sd) - pnorm(leftVals, mean, best_sd)
plot(xlim = c(-7, 7), ylim = c(0, 0.4), x = -1, y = -1)
points(x, probs, col="red", pch=20, lwd = 10)
points(x, frequencies / data_length, col="blue", pch=20, lwd = 2)

## With best sd and mean = prev mean chisq test works
chisq.test(frequencies, p=probs)
