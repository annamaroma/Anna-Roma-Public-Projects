close all;
clear all;
clc;

load("tracking_data.mat");

q = 15; % Number of sensors
n = 36; % State dimension
lambda = 40; % Changed from suggested 10
epsilon = 1;

state_errors_sso = zeros(1, T);
attack_errors_sso = zeros(1, T);
state_errors_dsso = zeros(1, T);
attack_errors_dsso = zeros(1, T);


% %% SSO
G = normalize([D, eye(q)]);
nu = norm(G)^-2;
k = 1;
% 
x_real = xtrue0;
a_real = atrue;
y_real = y;
x = zeros(n,T);
a = zeros(q,T);

<<<<<<< Updated upstream
=======
% clear("xtrue0");
% clear("atrue");
% clear("y");
 
>>>>>>> Stashed changes
supp_x_real = abs(x_real) > epsilon;
supp_a_real = abs(a_real) > epsilon;

for k=1:T
    y(:,k) = G(:,1:36) * x(:,k) + a(:,k);

    z = A * x(:,k) - (nu * A * G(:,1:36)' * (y(:,k) - y_real(:,k)));
    x(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);
    z = a(:,k) - (nu * G(:, 37:51) * (y(:,k) - y_real(:,k)));
    a(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);

    % measurements
    supp_x_est = abs(x(:,k)) > epsilon;
    state_errors_sso(k) = sum(supp_x_est) - sum(supp_x_real);

    supp_a_est = abs(a(:,k)) > epsilon;
    attack_errors_sso(k) = sum(supp_a_est) - sum(supp_a_real);
end

<<<<<<< Updated upstream
%% D-SSO
nu = 50; % As per task3 0.7
k = 1;

x_real = xtrue0;
a_real = atrue;
y_real = y;
x = zeros(n,T);
a = zeros(q,T);

for k=1:T
    y(:,k) = G(:,1:36) * x(:,k) + a(:,k);
    
    z = A * x(:,k) - (nu * A * G(:,1:36)' * (y(:,k) - y_real(:,k)));
    x(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);
    z = a(:,k) - (nu * G(:, 37:51) * (y(:,k) - y_real(:,k)));
    a(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);

    % measurements
    supp_x_est = abs(x(:,k)) > epsilon;
    state_errors_dsso(k) = sum(supp_x_est) - sum(supp_x_real);

    supp_a_est = abs(a(:,k)) > epsilon;
    attack_errors_dsso(k) = sum(supp_a_est) - sum(supp_a_real);
end

%% PLOTS
% Plotting the comparison for State Estimation Error
figure;
hold on;
plot(1:T, state_errors_sso, 'b', 'LineWidth', 2); 
plot(1:T, state_errors_dsso, 'r', 'LineWidth', 2); 
=======
% PLOTS
% Plotting the comparison for State Estimation Error
figure;
subplot(2,1,1);
plot(1:T, state_errors_sso, 'Color', [0.9290 0.6940 0.1250], 'LineWidth', 2); hold on;
>>>>>>> Stashed changes
xlabel('Iteration');
ylabel('State Estimation Error');
title('State Estimation Error: SSO vs DSSO');
legend('SSO', 'DSSO');
grid on;

<<<<<<< Updated upstream
% Plotting the comparison for Support Attack Error
figure;
hold on;
plot(1:T, max(attack_errors_sso, 0), 'b', 'LineWidth', 2); 
plot(1:T, max(attack_errors_dsso, 0), 'r', 'LineWidth', 2); 
=======
subplot(2,1,2);
plot(1:T, max(attack_errors_sso, 0), 'Color', [0.4940 0.1840 0.5560], 'LineWidth', 2); hold on;
>>>>>>> Stashed changes
xlabel('Iteration');
ylabel('Support Attack Error');
title('Support Attack Error: SSO vs DSSO');
legend('SSO', 'DSSO');
grid on;
ylim([0 1.4]);



%DSSO implementation
% D-SSO
nu = 90; 
k = 1;

x_real = xtrue0;
a_real = atrue;
y_real = y;
x = zeros(n,T);
a = zeros(q,T);

for k=1:T
    y(:,k) = G(:,1:36) * x(:,k) + a(:,k);

    z = A * x(:,k) - (nu * A * G(:,1:36)' * (y(:,k) - y_real(:,k)));
    x(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);
    z = a(:,k) - (nu * G(:, 37:51) * (y(:,k) - y_real(:,k)));
    a(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);

    % measurements
    supp_x_est = abs(x(:,k)) > epsilon;
    state_errors_dsso(k) = sum(supp_x_est) - sum(supp_x_real);

    supp_a_est = abs(a(:,k)) > epsilon;
    attack_errors_dsso(k) = sum(supp_a_est) - sum(supp_a_real);
end





% PLOTS

figure;
subplot(2,1,1);
plot(1:T, state_errors_dsso, 'Color', [0.9290 0.6940 0.1250], 'LineWidth', 2);
xlabel('Iteration');
ylabel('State Estimation Error');
title('State Estimation Error: DSSO');
legend('DSSO');
grid on;

subplot(2,1,2);
plot(1:T, max(attack_errors_dsso, 0), 'Color', [0.4940 0.1840 0.5560], 'LineWidth', 2);
xlabel('Iteration');
ylabel('Support Attack Error');
title('Support Attack Error: DSSO');
legend('DSSO');
grid on;



figure;
subplot(2,1,1);
plot(1:T, state_errors_sso, 'Color', [0 0.7 0.95], 'LineWidth', 2); hold on;
plot(1:T, state_errors_dsso, 'Color', [0.4660 0.6740 0.1880], 'LineWidth', 2);
xlabel('Iteration');
ylabel('State Estimation Error');
title('Confronto State Estimation Error: SSO vs DSSO');
legend('SSO', 'DSSO');
grid on;

subplot(2,1,2);
plot(1:T, max(attack_errors_sso, 0), 'Color', [0 0.7 0.95], 'LineWidth', 2); hold on;
plot(1:T, max(attack_errors_dsso, 0), 'Color', [0.4660 0.6740 0.1880], 'LineWidth', 2);
xlabel('Iteration');
ylabel('Support Attack Error');
title('Confronto Support Attack Error: SSO vs DSSO');
legend('SSO', 'DSSO');
grid on;

