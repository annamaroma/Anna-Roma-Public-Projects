close all;
clear all;
clc;

n = 15; % State dimension
q = 30; % Number of sensors
h = 2;  % Number of sensors under attack
sigma = 10^-2;
delta = 10^-10;
lambda = 0.1;

num_trials = 20;    % Number of runs
max_iters  = 20000; % Max cycles per run

error_IJAM = zeros(num_trials, 1);
error_ISTA = zeros(num_trials, 1);

SAE_IJAM_vals = zeros(num_trials, 1);
SAE_ISTA_vals = zeros(num_trials, 1);
identified_IJAM_vals = zeros(num_trials, 1);
identified_ISTA_vals = zeros(num_trials, 1);
true_attacks_vals = zeros(num_trials, 1);

rel_error_IJAM = zeros(max_iters, num_trials);
rel_error_ISTA = zeros(max_iters, num_trials);

for trial = 1:num_trials
    x_tilde = zeros(n,1); % Unknown state vector
    a_tilde = zeros(q,1); % Unknown sparse attack vector
    eta     = randn(q,1) .* (sigma^2); % Posssible measurement noise
    
    C = randn(q,n);
    
    supp = randperm(q,h); % Indexes of sensors under attack
    for index = 1:h
        if(rand() < 0.5)
            a_tilde(supp(index)) = rand()+4;
        else
            a_tilde(supp(index)) = rand()-5;
        end
    end
    
    for index = 1:n
        if(rand() < 0.5)
            x_tilde(index) = rand()+2;
        else
            x_tilde(index) = rand()-3;
        end
    end
    
    y = C * x_tilde + a_tilde + eta;
    C_pinv = pinv(C);

    %% IJAM
    nu_ijam = 0.7;
    x_ijam = zeros(n,2);
    a_ijam = zeros(q,2);
    k = 1;
    while true
        x_ijam(:,k+1) = C_pinv * (y - a_ijam(:,k));
        z = a_ijam(:,k) - nu_ijam * (C * x_ijam(:,k) + a_ijam(:,k) - y);
        a_ijam(:,k+1) = sign(z) .* max(abs(z) - lambda * nu_ijam, 0);

        rel_error_IJAM(k, trial) = norm(x_ijam(:, k+1) - x_tilde) / norm(x_tilde);

        if sum((x_ijam(:,k+1) - x_ijam(:,k)) .^ 2) < delta
            break;
        end

        x_ijam = [x_ijam, zeros(n,1)];
        a_ijam = [a_ijam, zeros(q,1)];
        k = k+1;
        if k > max_iters
            disp("IJAM runned for " + k + "iteration, no solution found, quitting!");
            break;
        end
    end
   
    %% ISTA
    G = [C, eye(q)];
    nu_ista = 0.99/(norm(G,2)^2);
    x_ista = zeros(n,2);
    a_ista = zeros(q,2);
    k = 1;
    while true
        x_ista(:,k+1) = x_ista(:,k) - nu_ista * C' * (C * x_ista(:,k)+ a_ista(:,k) - y);
        z = a_ista(:,k) - nu_ista * (C* x_ista(:,k) + a_ista(:,k) - y);
        a_ista(:,k+1) = sign(z) .* max(abs(z) - lambda * nu_ista, 0);

        rel_error_ISTA(k, trial) = norm(x_ista(:, k+1) - x_tilde) / norm(x_tilde);
    
        if sum((x_ista(:,k+1) - x_ista(:,k)) .^ 2) < delta
            break;
        end

        x_ista = [x_ista, zeros(n,1)];
        a_ista = [a_ista, zeros(q,1)];
        k = k+1;
        if k > max_iters
            disp("ISTA runned for " + k + "iteration, no solution found, quitting!");
            break;
        end
    end

    % Error estimation
    error_IJAM(trial) = norm(x_ijam(:, end) - x_tilde, 2) / norm(x_tilde, 2);
    error_ISTA(trial) = norm(x_ista(:, end) - x_tilde, 2) / norm(x_tilde, 2);

    % Counting how many sensors were found under attack
    true_support = (a_tilde ~= 0);
    estimated_support_IJAM = (a_ijam(:, end) ~= 0);
    estimated_support_ISTA = (a_ista(:, end) ~= 0);

    SAE_IJAM_vals(trial) = sum(abs(estimated_support_IJAM - true_support));
    SAE_ISTA_vals(trial) = sum(abs(estimated_support_ISTA - true_support));
    
    % Counting how many sensors were found
    true_attacks_vals(trial) = sum(true_support);
    identified_IJAM_vals(trial) = sum(estimated_support_IJAM);
    identified_ISTA_vals(trial) = sum(estimated_support_ISTA);
end

%% Results analysis
% Average and Std deviation
mean_error_IJAM = mean(error_IJAM);
std_error_IJAM = std(error_IJAM);
mean_error_ISTA = mean(error_ISTA);
std_error_ISTA = std(error_ISTA);

% Check of sensors under attack
mean_SAE_IJAM = mean(SAE_IJAM_vals);
mean_SAE_ISTA = mean(SAE_ISTA_vals);
std_SAE_IJAM = std(SAE_IJAM_vals);
std_SAE_ISTA = std(SAE_ISTA_vals);

mean_true_attacks = mean(true_attacks_vals);
mean_identified_IJAM = mean(identified_IJAM_vals);
mean_identified_ISTA = mean(identified_ISTA_vals);

%% Prints 
fprintf('\nFinal Results after %d runs:\n', num_trials);
fprintf('Mean Support Attack Error (IJAM): %.2f ± %.2f\n', mean_SAE_IJAM, std_SAE_IJAM);
fprintf('Mean Support Attack Error (ISTA): %.2f ± %.2f\n', mean_SAE_ISTA, std_SAE_ISTA);

fprintf('\nFinal Results after %d runs:\n', num_trials);
fprintf('------------------------------------------------\n');
fprintf('  Average true attacked sensors  : %.2f\n', mean_true_attacks);
fprintf('  IJAM identified sensors        : %.2f (SAE: %.2f ± %.2f)\n', ...
        mean_identified_IJAM, mean_SAE_IJAM, std_SAE_IJAM);
fprintf('  ISTA identified sensors        : %.2f (SAE: %.2f ± %.2f)\n', ...
        mean_identified_ISTA, mean_SAE_ISTA, std_SAE_ISTA);
fprintf('------------------------------------------------\n');

%Convergence rate
mean_rel_error_IJAM = mean(rel_error_IJAM, 2, 'omitnan');
mean_rel_error_ISTA = mean(rel_error_ISTA, 2, 'omitnan');

% Visualizzazione risultati
figure;
hold on;
bar([1, 2], [mean_error_IJAM, mean_error_ISTA]);
errorbar([1, 2], [mean_error_IJAM, mean_error_ISTA], [std_error_IJAM, std_error_ISTA], 'k', 'LineStyle', 'none');
xticks([1 2]);
xticklabels({'IJAM', 'ISTA'});
ylabel('Mean State Estimation Error');
title('Comparison of IJAM and ISTA Estimation Performance');
grid on;
hold off;


figure;
subplot(2,1,1);
stem(a_tilde, 'r', 'LineWidth', 1.5); hold on;
stem(a_ijam(:,end), 'b--', 'LineWidth', 1.2);
legend('True Anomaly', 'Estimated a (IJAM)');
title('IJAM: True vs Estimated Anomalies');

subplot(2,1,2);
stem(a_tilde, 'r', 'LineWidth', 1.5); hold on;
stem(a_ista(:,end), 'g--', 'LineWidth', 1.2);
legend('True Anomaly', 'Estimated a (ISTA)');
title('ISTA: True vs Estimated Anomalies');


figure;
semilogy(1:max_iters, mean_rel_error_IJAM, 'r', 'LineWidth', 2); hold on;
semilogy(1:max_iters, mean_rel_error_ISTA, 'b', 'LineWidth', 2);
xlabel('Iteration');
ylabel('Relative Error (log scale)');
title('Convergence Rate of ISTA vs IJAM');
legend('IJAM', 'ISTA');
grid on;