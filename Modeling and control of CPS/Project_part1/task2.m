close all;
clear all;
clc;

load("localization_data.mat");

n = 100; % Number of cells
q = 20;  % Number of sensors
lambda = 10;
delta = 10^-10;

G = normalize([D, eye(q)]);
nu = norm(G, 2)^-2;

max_iters  = 20000; % Max cycles per run

x = zeros(n,2);
a = zeros(q,2);

k = 1;
while true
    z = [x(:,k); a(:,k)] - nu * G' * (G * [x(:,k);a(:,k)] - y);
    x(:,k+1) = sign(z(1:n)) .* max(abs(z(1:n)) - lambda * nu, 0);
    a(:,k+1) = sign(z(n+1:q+n)) .* max(abs(z(n+1:q+n)) - lambda * nu, 0);

    if sum((x(:,k+1) - x(:, k)).^2) < delta
        break;
    end

    x = [x, zeros(n,1)];
    a = [a, zeros(q,1)];
    k = k + 1;
    if k > max_iters
        disp("ISTA runned for " + k + " iteration, no solution found, quitting!");
        break;
    end
end

supp_x = find(x(:,end))
supp_a = find(a(:,end))