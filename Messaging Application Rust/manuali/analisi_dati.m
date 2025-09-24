clc

%% analisi consumo CPU
[thisDir, ~, ~] = fileparts(mfilename('fullpath'));
logFile = fullfile(thisDir, 'analisi_dati_test3.log');

if ~isfile(logFile)
	error('File di log non trovato: %s', logFile);
end

txt = fileread(logFile);

pat = '\[(?<ts>.*?)\]\s*CPU:\s*(?<cpu>\d+(?:\.\d+)?)%';
tokens = regexp(txt, pat, 'names');

if isempty(tokens)
	error('Nessun dato CPU trovato nel file di log.');
end

tsRaw = {tokens.ts}.';              % cellstr (Nx1) con timestamp completi
cpuStr = {tokens.cpu}.';            % cellstr (Nx1) con numeri in stringa
cpu = str2double(cpuStr);           % double (Nx1)


tsClean = erase(tsRaw, ' UTC');
fmt = 'yyyy-MM-dd HH:mm:ss.SSSSSSSSS';
try
	t = datetime(tsClean, 'InputFormat', fmt, 'TimeZone', 'UTC');
catch
	tsNoFrac = extractBefore(tsClean, ' '); 
	tsToSec = regexprep(tsClean, '^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}).*$', '$1');
	t = datetime(tsToSec, 'InputFormat', 'yyyy-MM-dd HH:mm:ss', 'TimeZone', 'UTC');
end

[t, idx] = sort(t);
cpu = cpu(idx);

% line Plot
figure('Name','CPU % nel tempo','Color','w');
plot(t, cpu, '-o', 'LineWidth', 1.2, 'MarkerSize', 4, 'Color', [0.13 0.45 0.85]);
grid on;
title('Utilizzo CPU (%) nel tempo');
xlabel('Tempo (UTC)');
ylabel('CPU (%)');


yMin = max(0, min(cpu)-0.05);
yMax = max(1, max(cpu)+0.05);
ylim([yMin yMax]);
ax = gca; 

try
	xtickformat('HH:mm');
catch
	datetick('x', 'HH:MM', 'keepticks', 'keeplimits');
end
ax.XTickLabelRotation = 30;

fprintf('Campioni: %d  |  CPU min: %.2f%%  mediana: %.2f%%  media: %.2f%%  max: %.2f%%\n', ...
		numel(cpu), min(cpu), median(cpu), mean(cpu), max(cpu));

%bar plot
figure('Name','CPU % - Bar plot','Color','w');
bar(t, cpu, 'FaceColor', [0.13 0.45 0.85], 'EdgeColor', 'none');
grid on;
title('Utilizzo CPU (%) - Bar plot');
xlabel('Tempo (UTC)');
ylabel('CPU (%)');
ylim([yMin yMax]);
ax = gca;
try
	xtickformat('HH:mm');
catch
	datetick('x', 'HH:MM', 'keepticks', 'keeplimits');
end
ax.XTickLabelRotation = 30;


%% analisi consumo di memoria
% Estrae timestamp e MEM (in KB) dal testo del log già caricato in 'txt'
patMem = '\[(?<ts>.*?)\].*?MEM:\s*(?<mem>\d+)\s*KB';
tokensMem = regexp(txt, patMem, 'names');

if isempty(tokensMem)
	error('Nessun dato MEM trovato nel file di log.');
end

% Converte in vettori
tsRawM = {tokensMem.ts}.';      % timestamp grezzi
memKB = str2double({tokensMem.mem}.');
memMB = memKB / 1024;           % conversione in MB

% Parsing dei timestamp (riusa il formato 'fmt' definito sopra)
tsCleanM = erase(tsRawM, ' UTC');
try
	tM = datetime(tsCleanM, 'InputFormat', fmt, 'TimeZone', 'UTC');
catch
	tsToSecM = regexprep(tsCleanM, '^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}).*$', '$1');
	tM = datetime(tsToSecM, 'InputFormat', 'yyyy-MM-dd HH:mm:ss', 'TimeZone', 'UTC');
end

% Ordina per tempo
[tM, idxM] = sort(tM);
memMB = memMB(idxM);

% Line plot per memoria
figure('Name','Memoria (MB) nel tempo','Color','w');
plot(tM, memMB, '-o', 'LineWidth', 1.2, 'MarkerSize', 4, 'Color', [0.90 0.40 0.10]);
grid on;
title('Consumo Memoria (MB) nel tempo');
xlabel('Tempo (UTC)');
ylabel('Memoria (MB)');

% Limiti e formato asse X
yMinM = max(0, min(memMB) * 0.98);
yMaxM = max(yMinM + 1, max(memMB) * 1.02);
ylim([yMinM yMaxM]);
ax = gca;
try
	xtickformat('HH:mm');
catch
	datetick('x', 'HH:MM', 'keepticks', 'keeplimits');
end
ax.XTickLabelRotation = 30;

% Statistiche
fprintf('MEM campioni: %d | MB min: %.1f  mediana: %.1f  media: %.1f  max: %.1f\n', ...
		numel(memMB), min(memMB), median(memMB), mean(memMB), max(memMB));

% Bar plot per memoria
figure('Name','Memoria (MB) - Bar plot','Color','w');
bar(tM, memMB, 'FaceColor', [0.90 0.40 0.10], 'EdgeColor', 'none');
grid on;
title('Consumo Memoria (MB) - Bar plot');
xlabel('Tempo (UTC)');
ylabel('Memoria (MB)');
ylim([yMinM yMaxM]);
ax = gca;
try
	xtickformat('HH:mm');
catch
	datetick('x', 'HH:MM', 'keepticks', 'keeplimits');
end
ax.XTickLabelRotation = 30;


