close all;
clear all;
clc;

load("dynamic_CPS_data.mat");

q = 30; % Number of sensors
h = 3;  % Number of sensors under attack
n = 15; % State dimension
delta = 10^-10;
lambda = 0.1;
a0 = a;

max_iters  = 20000; % Max cycles per run

state_errors_sso = zeros(1, max_iters);
attack_errors_sso = zeros(1, max_iters);
state_errors_dsso = zeros(1, max_iters);
attack_errors_dsso = zeros(1, max_iters);

%% SSO
G = [C, eye(q)];
nu = 0.99 / (norm(G,2)^2);
k = 1;
tol = 0.1;

x_real = [x0, zeros(n,1)];
a_real = a0;
y_real = zeros(q,1);
x = zeros(n,2);
a = zeros(q,2);

while true
    y_real(:,k) = C * x_real(:,k) + a_real(:);
    y(:,k) = C * x(:,k) + a(:,k);
    x(:,k+1) = A * x(:,k) - (nu * A * C' * (y(:,k) - y_real(:,k)));
    z = a(:,k) - (nu * (y(:,k) - y_real(:,k)));
    a(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);
    x_real(:,k+1) = A * x_real(:,k);

    % measurements
    state_errors_sso(k) = norm(x(:,k) - x_real(:,k), 2) / norm(x_real(:,k), 2);

    a_true = abs(a_real) > tol;
    a_est = abs(a(:,k)) > tol;
    
    attack_errors_sso(k) = sum(abs(a_true - a_est));

    if sum((x(:,k+1) - x(:, k)).^2) < delta
        break;
    end

    x = [x, zeros(n,1)];
    a = [a, zeros(q,1)];

    k = k+1;
    if k > max_iters
        disp("SSO runned for " + k + " iteration, no solution found, quitting!");
        break;
    end
end
longer = k;

%% D-SSO
nu = 0.7;
k = 1;

x_real = [x0, zeros(n,1)];
a_real = a0;
y_real = zeros(q,1);
x = zeros(n,2);
a = zeros(q,2);

C_pinv = pinv(C);
L = A * C_pinv;

while true
    y_real(:,k) = C * x_real(:,k) + a_real(:);
    y(:,k) = C * x(:,k) + a(:,k);
    x(:,k+1) = A * x(:,k) - (L * (y(:,k) - y_real(:,k)));
    z = a(:,k) - (nu * (y(:,k) - y_real(:,k)));
    a(:,k+1) = sign(z) .* max(abs(z) - lambda * nu, 0);
    x_real(:,k+1) = A * x_real(:,k);

    % measurements
    state_errors_dsso(k) = norm(x(:,k) - x_real(:,k), 2) / norm(x_real(:,k), 2);

    a_true = abs(a_real) > tol;
    a_est = abs(a(:,k)) > tol;
    
    attack_errors_dsso(k) = sum(abs(a_true - a_est));

    if sum((x(:,k+1) - x(:, k)).^2) < delta
        break;
    end

    x = [x, zeros(n,1)];
    a = [a, zeros(q,1)];

    k = k+1;
    if k > max_iters
        disp("D-SSO runned for " + k + " iteration, no solution found, quitting!");
        break;
    end
end

if(k > longer)
    longer = k;
end

%% DATA POST PROCESSING
state_errors_sso = state_errors_sso(1:longer);
state_errors_dsso = state_errors_dsso(1:longer);
attack_errors_sso = attack_errors_sso(1:longer);
attack_errors_dsso = attack_errors_dsso(1:longer);

%% PLOTS
% % Plotting the comparison for State Estimation Error
% figure;
% set(gca, 'XScale', 'log')
% plot(1:longer, state_errors_sso, 'b', 'LineWidth', 2); hold on;
% plot(1:longer, state_errors_dsso, 'r', 'LineWidth', 2);
% xlabel('Iteration');
% ylabel('State Estimation Error');
% title('State Estimation Error: SSO vs DSSO');
% legend('SSO', 'DSSO');
% grid on;
% 
% % Plotting the comparison for Support Attack Error
% figure;
% set(gca, 'XScale', 'log')
% plot(1:longer, attack_errors_sso, 'b', 'LineWidth', 2); hold on;
% plot(1:longer, attack_errors_dsso, 'r', 'LineWidth', 2);
% xlabel('Iteration');
% ylabel('Support Attack Error');
% title('Support Attack Error: SSO vs DSSO');
% legend('SSO', 'DSSO');
% grid on;

% filepath: /home/annaroma/Scrivania/LAB_MODELING/ModelingLabs/task3.m
%% PLOTS
figure;
% State Estimation Error (left)
subplot(1,2,1);
set(gca, 'XScale', 'log')
plot(1:longer, state_errors_sso, 'Color', [0 0.7 0.95], 'LineWidth', 2); hold on;   % Cyan for SSO
plot(1:longer, state_errors_dsso, 'Color', [0.4660 0.6740 0.1880], 'LineWidth', 2); % Green for DSSO
xlabel('Iteration');
ylabel('State Estimation Error');
title('State Estimation Error: SSO vs DSSO');
legend('SSO', 'DSSO');
grid on;

% Support Attack Error (right)
subplot(1,2,2);
set(gca, 'XScale', 'log')
plot(1:longer, attack_errors_sso, 'Color', [0 0.7 0.95], 'LineWidth', 2); hold on;  % Cyan for SSO
plot(1:longer, attack_errors_dsso, 'Color', [0.4660 0.6740 0.1880], 'LineWidth', 2);% Green for DSSO
xlabel('Iteration');
ylabel('Support Attack Error');
title('Support Attack Error: SSO vs DSSO');
legend('SSO', 'DSSO');
grid on;



% Plotting the number of iterations necessary to converge for both algorithms
% find the first iteration where the support error attacks is zero, i.e., when the estimated support matches the true support
sso_idx_convergence = find(attack_errors_sso == 0, 1);
dsso_idx_convergence = find(attack_errors_dsso == 0, 1);

% Plotting the convergence iterations
figure;
b = bar([sso_idx_convergence, dsso_idx_convergence], 'FaceColor', 'flat');
b.CData(1,:) = [0 0.7 0.95];           % Cyan for SSO
b.CData(2,:) = [0.4660 0.6740 0.1880]; % Green for DSSO
set(gca, 'XTickLabel', {'SSO', 'DSSO'});
title('Convergence Iterations for SSO and DSSO');
grid on;

% Print the value on top of each bar for clarity
xt = get(gca, 'XTick');
yt = [sso_idx_convergence, dsso_idx_convergence];
for i = 1:length(yt)
    text(xt(i), yt(i) + 0.05*max(yt), num2str(yt(i)), ...
        'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'FontSize', 12);
end

