close all;

%% Plot the worst xj_tilde for each simulation
figure(1);
hold on;
title("xTilde");
for j=1:size(simOut, 1)
    subplot(size(simOut, 1)/2,2,j);
    hold on;
    title(simOut(j, 1).name);
    names = [];
    for i=1:6
        plot(simOut(j, 1).yout{1}.Values.Time, simOut(j, 1).yout{1}.Values.Data(:,(i*2)-1));
        plot(simOut(j, 1).yout{1}.Values.Time, simOut(j, 1).yout{1}.Values.Data(:,(i*2)));
        names = [names, "Pos " + i, "Vel " + i];
    end
    legend(names);
    ylim([-5, 5]);
end

return;

%% Plot the worst xj_tilde for each node
for j=1:size(simOut, 1)
    figure();
    hold on;
    title(simOut(j, 1).name);
    for i=1:6
        subplot(3,2,i);
        hold on;
        title("x_" + i + "Tilde");
        names = [];
        plot(simOut(j, 1).yout{1}.Values.Time, simOut(j, 1).yout{1}.Values.Data(:,i));
    end
end
