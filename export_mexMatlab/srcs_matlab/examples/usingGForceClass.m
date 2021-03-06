%usingGForceClass is an examples that uses the GForce class. This is a
%class that handles some of the

%{
Laboratorio de Inteligencia y Visión Artificial
ESCUELA POLITÉCNICA NACIONAL
Quito - Ecuador

autor: ztjona!
jonathan.a.zea@ieee.org

"I find that I don't understand things unless I try to program them."
-Donald E. Knuth

05 March 2021
Matlab 9.9.0.1592791 (R2020b) Update 5.
%}

clc
close all

%% NOTE
% mex function must be added to the path.

%% creating object
options.emgResolution = 8; % 8 or 12
options.enabledQuats = true;
options.emgFreq = 500;
options.verbose = 0; % 0 no, 1 yes

gf = GForce(options);

%% mai methods

% -- Getting battery
bat = gf.getBattery();
fprintf('\n\nDevice battery is %d\n\n', bat)

% -- Verbose
% Create another object.

% -- Vibrating, ms
gf.vibrate(300);

% -- getting Emg
data = gf.getEmg();
if ~isempty(data)
    fprintf('\n\nEmg data size: (%d, %d)\n\n', size(data))
    plot(data(3, :)),title('Emg: channel 3')
else
    warning('Emg data not retrieved')
end

dataQ = gf.getOrientation();
if ~isempty(dataQ)
    fprintf('\n\nQuaternions data size: (%d, %d)\n\n', size(dataQ))
    figure,plot(dataQ'),title('Quaternions'),legend('w','x','y','z')
else
    warning('Quaternions data not retrieved')
end

% -- changing Emg resolution
% Create another object.