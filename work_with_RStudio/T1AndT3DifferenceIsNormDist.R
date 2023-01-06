library( MASS )

## Function for getting data (it's discrete) from experiment with our model
get_discrete_data = function() {
  value_count <- c(28, 211, 947, 2306, 3061, 2268, 902, 245, 31, 1)
  values <- -4:5
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

## Function for finding parameters for our data when we except that it's norm distribution
find_distibution_parameters = function(data) {
  parameters = fitdistr(data, "normal")
  return (parameters$estimate)
}

## Find parameters for discrete data
discrete_data_parameters = find_distibution_parameters(discrete_data)
discrete_data_mean = discrete_data_parameters[1]
discrete_data_sd = discrete_data_parameters[2]

## Function for checking a simple hypothesis with ks.test
check_simple_hypothesis = function(data, data_parameteres) {
  ks.test(data, "pnorm", mean = data_parameteres[1], sd = data_parameteres[2])
}

## Check a simple hypothesis with ks.test. Reject it (because of the discrete data)
check_simple_hypothesis(discrete_data, discrete_data_parameters)

## Generate vector of norm distribution values
genetate_norm_distribution_data = function(data_length, data_parameteres) {
  data = rnorm(data_length, mean = data_parameteres[1], sd = data_parameteres[2])
}
norm_data_length = discrete_data_length
norm_data_parameters = discrete_data_parameters
norm_data = genetate_norm_distribution_data(norm_data_length, norm_data_parameters)
print_hist(norm_data)

## Print two histograms. Not good Graphics.
## We need to round norm distribution or to get continuous data 
print_two_histogram(discrete_data, norm_data)

## Function for getting round data from continuous data
get_round_data = function(data) {
  return (round(data))
}
round_norm_data = get_round_data(norm_data)
print_two_histogram(discrete_data, round_norm_data)
## Check that norm data is norm distribution, but round norm data is not!
check_simple_hypothesis(norm_data, norm_data_parameters)
check_simple_hypothesis(round_norm_data, norm_data_parameters)

## Function for getting continuous data from discrete data.
## Generate norm distribution and get fractional part of it,
## and then add it to our data
get_continuous_data = function(data, data_length, data_parameters) {
  norm_data = sort(genetate_norm_distribution_data(data_length, data_parameters))
  round_norm_data = get_round_data(norm_data)
  added_values = norm_data - round_norm_data
  data = sort(data)
  
  continuous_data = data + added_values
  return (continuous_data)
}
continious_data = get_continuous_data(discrete_data, discrete_data_length, discrete_data_parameters)
print_two_histogram(continious_data, norm_data)
## Check that norm data is norm distribution and continuous data is norm too!
## (But we can improve parameters finding)
check_simple_hypothesis(norm_data, norm_data_parameters)
check_simple_hypothesis(continious_data, norm_data_parameters)
## Check that continuous round norm data is norm distribution.
## It proves that our method is work.
continuous_round_norm_data = get_continuous_data(round_norm_data, norm_data_length, norm_data_parameters)
print_two_histogram(norm_data, continuous_round_norm_data)
check_simple_hypothesis(norm_data, norm_data_parameters)
check_simple_hypothesis(continuous_round_norm_data, norm_data_parameters)

## Now we need to find correct parameters for our data
## Function for printing ideal density of the norm distribution and
## the density of our continuous data that was discrete
print_densities = function(data, data_length, data_parameters) {
  continuous_data = get_continuous_data(data, data_length, data_parameters)
  
  par(mfrow = c(1, 1))
  x = seq(-5, 5, by=0.1)
  y = dnorm(x, mean = data_parameters[1], sd = data_parameters[2])
  plot(x, y, col = "transparent")
  lines(x, y, col = "black")
  
  norm_data = genetate_norm_distribution_data(data_length, data_parameters)
  den <- density(norm_data)
  lines(den$x, den$y, col = "blue")
  
  den <- density(continious_data)
  lines(den$x, den$y, col = "red")
  
  return (c(continuous_data, norm_data))
}

## Function for finding maximum p-value depend on data_sd
optimize_data_sd = function(cur_data_sd) {
  cur_data_mean = 0
  cur_data_parameters = c(cur_data_mean, cur_data_sd)
  cur_continuous_data = get_continuous_data(discrete_data, discrete_data_length, cur_data_parameters)
  cur_res = check_simple_hypothesis(cur_continuous_data, cur_data_parameters)
  return (cur_res$p.value)
}

optimuze_result_vec = c()
for (i in 1:100) {
  optimize_result = optimize(optimize_data_sd, interval = c(1, 2), maximum = TRUE, tol = 0.0001)
  optimuze_result_vec[i] = optimize_result$maximum
}
optimize_result = mean(optimuze_result_vec)
optimize_result

## Use finding data_sd and check it
cur_data_mean = 0
cur_data_sd = optimize_result
cur_data_parameters = c(cur_data_mean, cur_data_sd)
dates = print_densities(discrete_data, discrete_data_length, cur_data_parameters)
cur_continuous_data = dates[1:discrete_data_length]
cur_norm_data = dates[(discrete_data_length + 1):(2 * discrete_data_length)]

## Check that norm data is norm distribution
## and our data is norm distribution too
print_two_histogram(cur_continuous_data, cur_norm_data)
check_simple_hypothesis(cur_norm_data, cur_data_parameters)
check_simple_hypothesis(cur_continuous_data, cur_data_parameters)
