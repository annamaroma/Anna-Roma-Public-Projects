close all;
clear all;
clc;

load("localization_data.mat");

n = 100; % Number of cells
q = 20;  % Number of sensors
lambda = 1;
delta = 10^-8;
epsilon = 1;

G = normalize ([D, eye(q)]);
nu = 0.01 * norm(G, 2)^-2;

max_iters = 200000; % Max cycles per run

%% RING TOPOLOGY
load("Q_ring.mat");

z = zeros(n+q, q); % [x a], from the pov of each sensor

k = 1;
while true
    prev_z = z;
    for i=1:q % Compute the calculation from the pov of each sensor
        tmp_sum = 0;
        for j=1:q
            tmp_sum = tmp_sum + Q(i, j) * prev_z(:,j);
        end
        tmp_z = tmp_sum + nu * G(i,:)' * (y(i) - G(i,:) * prev_z(:, i));
        z(:, i) = sign(tmp_z) .* max(abs(tmp_z) - lambda * nu, 0);
    end

    tmp_sum = 0;
    for i=1:q
        tmp_sum = tmp_sum + norm(z(:,i) - prev_z(:,i))^2;
    end
    % tmp_sum
    if tmp_sum < delta
        break;
    end

    k = k + 1;
    if k > max_iters
        disp("DISTA runned for " + k + " iteration, no solution found, quitting!");
        break;
    end
end

% Data post process
x = z(1:n, :);
for k = 1:q
    x(abs(x(:, k))<max(x(:, k)), k) = 0;
end

a = z(n+1:end, :);
a(abs(a)<epsilon) = 0;


result_x = all(x ~= 0, 2);
result_a = all(a ~= 0, 2);

% Results
disp("Ring topology simulation ended");
if any(result_x)
    disp('Consensus on state found');
    disp(find(result_x));
else
    disp('Consensus on state NOT found');
end

if any(result_a)
    disp('Consensus on attack found');
    disp(find(result_a));
else
    disp('Consensus on attack NOT found');
end

%% START TOPOLOGY
load ("Q_star.mat");

z = zeros(n+q, q); % [x a], from the pov of each sensor


k = 1;
while true
    prev_z = z;

    z0 = zeros(n+q,1);
    for j=1:q
        z0 = z0 + Q(1,j+1)*z(:,j);
    end

    for i=1:q % Compute the calculation from the pov of each sensor
        tmp_z = z0 + nu * G(i,:)' * (y(i) - G(i,:) * prev_z(:, i));
        z(:, i) = sign(tmp_z) .* max(abs(tmp_z) - lambda * nu, 0);
    end
    
    tmp_sum = 0;
    for i=1:q
        tmp_sum = tmp_sum + norm(z(:,i) - prev_z(:,i))^2;
    end
    % tmp_sum
    if tmp_sum < delta
        break;
    end

    k = k + 1;
    if k > max_iters
        disp("DISTA runned for " + k + " iteration, no solution found, quitting!");
        break;
    end
end
% Data post process
x = z(1:n, :);
for k = 1:q
    x(abs(x(:, k))<max(x(:, k)), k) = 0;
end

a = z(n+1:end, :);
a(abs(a)<epsilon) = 0;


result_x = all(x ~= 0, 2);
result_a = all(a ~= 0, 2);

% Results
disp("Star topology simulation ended");
if any(result_x)
    disp('Consensus on state found');
    disp(find(result_x));
else
    disp('Consensus on state NOT found');
end

if any(result_a)
    disp('Consensus on attack found');
    disp(find(result_a));
else
    disp('Consensus on attack NOT found');
end