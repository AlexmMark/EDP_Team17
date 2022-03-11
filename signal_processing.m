data = csvread('alex.csv');
new_data = zeros(length(data));
a = 0.5;
new_data(1) = a * data(1);
for num = 2:length(data)
    new_data(num) = a * data(num) + (1-a)*new_data(num-1);
end

hold on;
figure;
plot(new_data);
figure;
plot(data);
hold off;