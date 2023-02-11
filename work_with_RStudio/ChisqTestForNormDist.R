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
plot(x, probs, col="green", pch=4)
points(x, frequencies / data_length, col="blue", pch=20)

## With best sd and mean = 0 chisq test works
chisq.test(frequencies, p=probs)
