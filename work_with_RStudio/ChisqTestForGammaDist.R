library( MASS )

## Function for getting data (it's discrete) from experiment with our model
get_data = function() {
  frequencies <- c(17, 110, 560, 1343, 2028, 2223, 1762, 1143, 541, 197, 58, 18)
  values <- seq(13, 18.5, 0.5)
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

## Compute shape and rate
parameters = fitdistr(data, "gamma")$estimate
shape = parameters[1]
rate = parameters[2]
mean = mean(data)
shape
rate
mean

## Get probabilities for the intervals
rightVals = c(seq(13.25, 18.25, 0.5), Inf)
leftVals = c(-Inf, seq(13.25, 18.25, 0.5))
x = seq(13, 18.5, 0.5)
probs = pgamma(rightVals, shape, rate) - pgamma(leftVals, shape, rate)

## Chisq test does not work with sample shape and rate
chisq.test(frequencies, p=probs)

## Optimize p value and find best shape (sample mean = 15.4969)
optimize_data_shape = function(cur_data_shape) {
  cur_data_rate = cur_data_shape / mean
  probs = (pgamma(rightVals, cur_data_shape, cur_data_rate) - 
             pgamma(leftVals, cur_data_shape, cur_data_rate))
  cur_res = chisq.test(frequencies, p=probs)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_shape, interval = c(300, 320),
                           maximum = TRUE, tol = 0.0001)
optimize_shape = optimize_result$maximum
optimize_shape
optimize_rate = optimize_shape / mean
optimize_rate

probs = (pgamma(rightVals, optimize_shape, optimize_rate) 
         - pgamma(leftVals, optimize_shape, optimize_rate))
plot(x, probs, col="green", pch=4)
points(x, frequencies / data_length, col="blue", pch=20)

## With best shape and sample mean chisq test does not work
chisq.test(frequencies, p=probs)

##########################################################################

## Same tests for Bartlett P1 and P2 flows
## Function for getting data (it's discrete) from experiment with our model
get_data = function() {
  frequencies <- c(11, 25, 85, 243, 400, 685, 963, 1226, 
                   1308, 1288, 1160, 944, 635, 465, 241, 
                   186, 79, 33, 16, 7)
  values <- seq(17.5, 27, 0.5)
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

## Compute shape and rate
parameters = fitdistr(data, "gamma")$estimate
shape = parameters[1]
rate = parameters[2]
mean = mean(data)
shape
rate
mean

## Get probabilities for the intervals
rightVals = c(seq(17.75, 26.75, 0.5), Inf)
leftVals = c(-Inf, seq(17.75, 26.75, 0.5))
x = seq(17.5, 27, 0.5)
probs = pgamma(rightVals, shape, rate) - pgamma(leftVals, shape, rate)

## Chisq test does not work with sample shape and rate
chisq.test(frequencies, p=probs)

## Optimize p value and find best shape (sample mean = 15.4969)
optimize_data_shape = function(cur_data_shape) {
  cur_data_rate = cur_data_shape / mean
  probs = (pgamma(rightVals, cur_data_shape, cur_data_rate) - 
             pgamma(leftVals, cur_data_shape, cur_data_rate))
  cur_res = chisq.test(frequencies, p=probs)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_shape, interval = c(210, 220),
                           maximum = TRUE, tol = 0.0001)
optimize_shape = optimize_result$maximum
optimize_shape
optimize_rate = optimize_shape / mean
optimize_rate

probs = (pgamma(rightVals, optimize_shape, optimize_rate) 
         - pgamma(leftVals, optimize_shape, optimize_rate))
plot(x, probs, col="green", pch=4)
points(x, frequencies / data_length, col="blue", pch=20)

## With best shape and sample mean chisq test does not work
chisq.test(frequencies, p=probs)