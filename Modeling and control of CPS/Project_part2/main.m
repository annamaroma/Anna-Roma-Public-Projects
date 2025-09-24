clear all
clc
close all

global N n A B C D Q R K F Adjency L G lambda c F_local

N = 6;
n = 2;
A = [0 1; 880.87 0];
B = [0; -9.9453];
B_pinv = pinv(B);
C = [708.27 0];
D = 0;

Q = eye(n);
R = 1;

P_k = are(A, B*inv(R) * B', Q);
K = inv(R) * B' * P_k;
P_f = are(A', C' * inv(R) * C, Q)';
F = P_f * C' * inv(R);

% Adjency = [0 0 0 0 0 0; 2 0 0 0 0 0; 0 6 0 0 0 0; 0 0 1 0 0 0; 0 0 0 1 0 0; 0 0 0 0 2 0];
% D = diag([0,2,6,1,1,2]);
% L = D-Adjency;
% G = diag([1,0,0,0,0,0]);

% waterfall topology pesi [2,1,6,1,3,2]
% Adjency = [0 0 0 0 0 0; 0 0 0 0 0 0; 0 6 0 0 0 0; 0 1 0 0 0 0; 0 0 3 0 0 0; 0 0 2 0 0 0];
% D = diag([2,1,6,1,3,2]);
% L = D-Adjency;
% G= diag([2,1,0,0,0,0]);

% % waterfall topology pesi [2,2,4,1,4,1]
% Adjency = [0 0 0 0 0 0; 0 0 0 0 0 0; 0 4 0 0 0 0; 0 1 0 0 0 0; 0 0 4 0 0 0; 0 0 1 0 0 0];
% D = diag([2,2,4,1,4,1]);
% L = D-Adjency;
% G= diag([2,2,0,0,0,0]);

Adjency = [0 0 0 0 0 0; 0 0 0 0 0 0; 0 1 0 0 0 0; 0 2 0 0 0 0; 0 0 2 0 0 0; 0 0 1 0 0 0];
D = diag([0,0,1,2,2,1]);
L = D-Adjency;
G= diag([6,3,0,0,0,0]);


lambda = eig(L+G);
c = 1/(2* min(real(lambda)));
c = c*3;

S0_x0=[10 0]';
Si_x0=[0 0]';

desired_poles = [-5, -6];        % Poli desiderati dell'osservatore
L_luemb = place(A', C', desired_poles)';

x_ref=[3;0];

desired_poles = [-5, -6];        % Poli desiderati dell'osservatore
L_local = place(A', C', desired_poles)'; % L guadagno osservatore standard
F_local = -L_local / c;


p_desired_origin = [-6; -7]; %track 0
p_desired_tracking = [0 + 3i, 0 - 3i]; %sin
p_desired_constant = [0; -0.0001]; 
%p_desired_ramp = [0; ]; 
K0 = place(A, B, p_desired_tracking);


% open('SVFB.slx');
% y = sim('SVFB.slx');

%open('SVFB_LocalObserver.slx');
%sim('SVFB_LocalObserver.slx');

% %%
% 
% t = y.x1.Time;
% e1 = squeeze(y.x1.Data);  % riduce da Nx1x1 → Nx1
% e2 = squeeze(y.x2.Data);
% e3 = squeeze(y.x3.Data);
% e4 = squeeze(y.x4.Data);
% e5 = squeeze(y.x5.Data);
% e6 = squeeze(y.x6.Data);
% 
% 
% figure;
% plot(t, e1(1,:), 'r', 'LineWidth', 1.3); hold on;
% plot(t, e2(1,:), 'b', 'LineWidth', 1.3);
% plot(t, e3(1,:), 'g', 'LineWidth', 1.3);
% plot(t, e4(1,:), 'm', 'LineWidth', 1.3);
% plot(t, e5(1,:), 'y', 'LineWidth', 1.3);
% plot(t, e6(1,:), 'k', 'LineWidth', 1.3);
% grid on;
% legend('position error node 1', 'position error node 2', 'position error node 3', 'position error node 4',  'position error node 5', 'position error node 6');
% xlabel('Time [s]');
% ylabel('State Error');
% title('Evolution of the state error');

% %% STATIC REFERENCE
% globalObs_static = simOut(1,:).yout{3}.Values.Data(:, 1:2:end);
% localObs_static = simOut(2,:).yout{3}.Values.Data(:, 1:2:end);
% 
% figure
% t = tiledlayout(1,2, 'Padding', 'compact', 'TileSpacing', 'compact');
% 
% 
% nexttile
% plot(globalObs_static, 'LineWidth', 2)
% xlabel('Time')
% ylabel('Error')
% legend('Error on node 1','Error on node 2','Error on node 3','Error on node 4','Error on node 5','Error on node 6')
% title('Global Cooperative Observer')
% grid on
% 
% nexttile
% plot(localObs_static, 'LineWidth', 2)
% xlabel('Time')
% ylabel('Error')
% legend('Error on node 1','Error on node 2','Error on node 3','Error on node 4','Error on node 5','Error on node 6')
% title('Local Observer')
% grid on
% 
% t.TileSpacing = 'compact'; % per ridurre lo spazio tra i grafici
% t.Padding = 'compact';     % per ridurre margini esterni
% 
% %% SINUSOIDAL REFERENCE
% globalObs_sin = simOut(3,:).yout{3}.Values.Data(:, 1:2:end);
% localObs_sin = simOut(4,:).yout{3}.Values.Data(:, 1:2:end);
% 
% figure
% t = tiledlayout(1,2, 'Padding', 'compact', 'TileSpacing', 'compact');
% 
% nexttile
% plot(globalObs_sin, 'LineWidth', 2)
% xlabel('Time')
% ylabel('Error')
% legend('Error on node 1','Error on node 2','Error on node 3','Error on node 4','Error on node 5','Error on node 6')
% title('Global Cooperative Observer')
% grid on
% 
% nexttile
% plot(localObs_sin, 'LineWidth', 2)
% xlabel('Time')
% ylabel('Error')
% legend('Error on node 1','Error on node 2','Error on node 3','Error on node 4','Error on node 5','Error on node 6')
% title('Local Observer')
% grid on
% 
% t.TileSpacing = 'compact'; % per ridurre lo spazio tra i grafici
% t.Padding = 'compact';     % per ridurre margini esterni
% 
% %% INCREMENTAL REFERENCE
% globalObs_inc = simOut(5,:).yout{3}.Values.Data(:, 1:2:end);
% localObs_inc = simOut(6,:).yout{3}.Values.Data(:, 1:2:end);
% 
% figure
% t = tiledlayout(1,2, 'Padding', 'compact', 'TileSpacing', 'compact');
% 
% nexttile
% plot(globalObs_inc, 'LineWidth', 2)
% xlabel('Time')
% ylabel('Error')
% legend('Error on node 1','Error on node 2','Error on node 3','Error on node 4','Error on node 5','Error on node 6')
% title('Global Cooperative Observer')
% grid on
% 
% nexttile
% plot(localObs_inc, 'LineWidth', 2)
% xlabel('Time')
% ylabel('Error')
% legend('Error on node 1','Error on node 2','Error on node 3','Error on node 4','Error on node 5','Error on node 6')
% title('Local Observer')
% grid on
% 
% t.TileSpacing = 'compact'; % per ridurre lo spazio tra i grafici
% t.Padding = 'compact';     % per ridurre margini esterni