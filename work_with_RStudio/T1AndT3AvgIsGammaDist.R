library( MASS )

## Function for getting data (it's discrete) from experiment with our model
get_discrete_data = function() {
  value_count <- c(17, 110, 560, 1343, 2028, 2223, 1762, 1143, 541, 197, 58, 14, 3, 1)
  values <- seq(13, 19.5, 0.5)
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
## Need to round 13 13.5 14 14.5 15...
get_round_data = function(data) {
  data_x2 = data * 2
  round_data_x2 = round(data_x2)
  round_data = round_data_x2 / 2.0
  return (round_data)
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
## It proves that our method is work.
continuous_round_gamma_data = get_continuous_data(round_gamma_data, gamma_data_length, gamma_data_parameters)
print_two_histogram(gamma_data, continuous_round_gamma_data)
check_simple_hypothesis(gamma_data, gamma_data_parameters)
check_simple_hypothesis(continuous_round_gamma_data, gamma_data_parameters)

## Now we need to find correct parameters for our data
## Function for printing ideal density of the gamma distribution and
## the density of our continuous data that was discrete
print_densities = function(data, data_length, data_parameters) {
  continuous_data = get_continuous_data(data, data_length, data_parameters)
  
  par(mfrow = c(1, 1))
  x = seq(10, 22, by=0.1)
  y = dgamma(x, shape = data_parameters[1], rate = data_parameters[2])
  plot(x, y, col = "transparent")
  lines(x, y, col = "black")
  
  gamma_data = genetate_gamma_distribution_data(data_length, data_parameters)
  den <- density(gamma_data)
  lines(den$x, den$y, col = "blue")
  
  den <- density(continious_data)
  lines(den$x, den$y, col = "red")
  
  return (c(continuous_data, gamma_data))
}

## Function for finding maximum p-value depend on data_shape
optimize_data_shape = function(cur_data_shape) {
  cur_data_rate = cur_data_shape / 15.5
  cur_data_parameters = c(cur_data_shape, cur_data_rate)
  cur_continuous_data = get_continuous_data(discrete_data, discrete_data_length, cur_data_parameters)
  cur_res = check_simple_hypothesis(cur_continuous_data, cur_data_parameters)
  return (cur_res$p.value)
}

optimuze_result_vec = c()
for (i in 1:100) {
  optimize_result = optimize(optimize_data_shape, interval = c(300, 320), maximum = TRUE, tol = 0.0001)
  optimuze_result_vec[i] = optimize_result$maximum
}
optimize_result = mean(optimuze_result_vec)
optimize_result

## Use finding data_shape and check it
cur_data_shape = optimize_result
cur_data_rate = cur_data_shape / 15.5
cur_data_parameters = c(cur_data_shape, cur_data_rate)
dates = print_densities(discrete_data, discrete_data_length, cur_data_parameters)
cur_continuous_data = dates[1:discrete_data_length]
cur_gamma_data = dates[(discrete_data_length + 1):(2 * discrete_data_length)]

## Check that gamma data is gamma distribution, but continuous data is not!
## It is better than ever, but p-value is still too small. 
print_two_histogram(cur_continuous_data, cur_gamma_data)
check_simple_hypothesis(cur_gamma_data, cur_data_parameters)
check_simple_hypothesis(cur_continuous_data, cur_data_parameters)

## Round gamma dist and our data have the same distribution
print_two_histogram(discrete_data, round_gamma_data)
ks.test(discrete_data, round_gamma_data)

## Get much more values in round gamma data and check it
length = 1000000
larger_gamma_data = genetate_gamma_distribution_data(length, cur_data_parameters)
round_larger_gamma_data = get_round_data(larger_gamma_data)
ks.test(discrete_data, round_larger_gamma_data)

## Gamma dist and our continuous data have the same distribution
print_two_histogram(cur_continuous_data, cur_gamma_data)
ks.test(cur_continuous_data, cur_gamma_data)

## Ñheck it with much more values in gamma data (not as bad as it could be)
ks.test(cur_continuous_data, larger_gamma_data)
