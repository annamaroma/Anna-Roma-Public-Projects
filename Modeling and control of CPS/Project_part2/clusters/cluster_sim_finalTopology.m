clear all
clc
close all

Tf = 1;
simOut = [];
open('SVFB.slx');
open('SVFB_LocalObserver.slx');

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


Adjency = [0 0 0 0 0 0; 0 0 0 0 0 0; 0 4 0 0 0 0; 0 1 0 0 0 0; 0 0 4 0 0 0; 0 0 1 0 0 0];
D = diag([0,0,4,1,4,1]);
L = D-Adjency;
G= diag([2,2,0,0,0,0]);


lambda = eig(L+G);
c = 1/(2* min(real(lambda)));
c = c*3;

S0_x0=[10 0]';
Si_x0=[0 0]';

desired_poles = [-5, -6];        % Poli desiderati dell'osservatore
L_luemb = place(A', C', desired_poles)';

err = 100;

%% SIM Const
x_ref=[5;0];

desired_poles = [-5, -6];        % Poli desiderati dell'osservatore
L_local = place(A', C', desired_poles)'; % L guadagno osservatore standard
F_local = -L_local / c;


p_desired_origin = [-6; -7]; %track 0
%p_desired_tracking = [0 + 3i, 0 - 3i]; %sin
p_desired_constant = [0; -0.0001]; %const
K0 = place(A, B, p_desired_constant);

tmpOut = sim('SVFB.slx');
tmpOut.name = "ConstObserver";
simOut = [simOut; tmpOut];
disp("Finished " + tmpOut.name + "...");
tmpOut = sim('SVFB_LocalObserver.slx');
tmpOut.name = "ConstLocalObserver";
simOut = [simOut; tmpOut];
disp("Finished " + tmpOut.name + "...");

%% SIM Ramp
x_ref=[5;10];

desired_poles = [-5, -6];        % Poli desiderati dell'osservatore
L_local = place(A', C', desired_poles)'; % L guadagno osservatore standard
F_local = -L_local / c;


p_desired_origin = [-6; -7]; %track 0
%p_desired_tracking = [0 + 3i, 0 - 3i]; %sin
p_desired_constant = [0; -0.0001]; %const
K0 = place(A, B, p_desired_constant);

tmpOut = sim('SVFB.slx');
tmpOut.name = "RampObserver";
simOut = [simOut; tmpOut];
disp("Finished " + tmpOut.name + "...");
tmpOut = sim('SVFB_LocalObserver.slx');
tmpOut.name = "RampLocalObserver";
simOut = [simOut; tmpOut];
disp("Finished " + tmpOut.name + "...");

%% SIM Sin
x_ref=[3;0];

desired_poles = [-5, -6];        % Poli desiderati dell'osservatore
L_local = place(A', C', desired_poles)'; % L guadagno osservatore standard
F_local = -L_local / c;


p_desired_origin = [-6; -7]; %track 0
p_desired_tracking = [0 + 3i, 0 - 3i]; %sin
%p_desired_constant = [0; -0.0001]; %const
K0 = place(A, B, p_desired_tracking);

tmpOut = sim('SVFB.slx');
tmpOut.name = "SinObserver";
simOut = [simOut; tmpOut];
disp("Finished " + tmpOut.name + "...");
tmpOut = sim('SVFB_LocalObserver.slx');
tmpOut.name = "SinLocalObserver";
simOut = [simOut; tmpOut];
disp("Finished " + tmpOut.name + "...");

%% POST PROCESS
save("simOut.mat", "simOut");
plot_cluster_sim;