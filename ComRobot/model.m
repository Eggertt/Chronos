raw = load('-ascii', 'Log_cloud2.txt');
#first = 112;
#last = 267;
first = 141;
last = 236;
yr = raw(first:last, 3);
yr = yr - mean(yr)
#yr = yr / (5 * std(yr))
#plot(yr)
s = raw(first:last, 5:9);
#plot(s)
Y = yr;
X = [ones(last - first + 1, 1), s];
X = [X, X(:, 2) .* X(:, 3), X(:, 3) .* X(:, 4), X(:, 4) .* X(:, 5), X(:, 5) .* X(:, 6)];

K = (X' * X)^-1 * (X' * Y)

Ye = X * K

plot([Y, Ye])

mean(abs(Y - Ye))

#all = [Y, X];
#save('-ascii', 'data1.txt', 'all')