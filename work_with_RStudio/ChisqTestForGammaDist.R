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
shape
rate

## Get probabilities for the intervals
rightVals = c(seq(13.25, 18.25, 0.5), Inf)
leftVals = c(-Inf, seq(13.25, 18.25, 0.5))
x = seq(13, 18.5, 0.5)
probs = pgamma(rightVals, shape, rate) - pgamma(leftVals, shape, rate)

## Chisq test does not work with sample shape and rate
chisq.test(frequencies, p=probs)

## Optimize p value and find best shape (sample mean = 15.4969)
optimize_data_shape = function(cur_data_shape) {
  mean = mean(data)
  cur_data_rate = cur_data_shape / mean
  probs = (pgamma(rightVals, cur_data_shape, cur_data_rate) - 
             pgamma(leftVals, cur_data_shape, cur_data_rate))
  cur_res = chisq.test(frequencies, p=probs)
  return (cur_res$p.value)
}

optimize_result = optimize(optimize_data_shape, interval = c(300, 320),
                           maximum = TRUE, tol = 0.0001)
best_shape = optimize_result$maximum
best_shape
best_rate = best_shape / mean(data)

probs = pgamma(rightVals, best_shape, best_rate) - pgamma(leftVals, best_shape, best_rate)
plot(x, probs, col="green", pch=4)
points(x, frequencies / data_length, col="blue", pch=20)

## With best shape and sample mean chisq test does not work
chisq.test(frequencies, p=probs)
