%basicGForce is an example that tests GForce is working

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
%% libs
% Trying to add '\export_mexMatlab\mex\gForce_mex.mexw64' to the path.
% Adding the folder based on current dir.

currentDir = pwd;
folder = strsplit(currentDir, '\');
switch folder{end}
    case 'examples'
        addpath(genpath('..\..\mex\'))
        
    case 'srcs_matlab'
        addpath(genpath('..\mex\'))
        
    case 'export_mexMatlab'
        addpath(genpath('.\mex\'))
end



% --- Checking that gForce_mex is added to the path
switch exist('gForce_mex', 'file')
    case 3
        disp('Mex function correctly found!')
    otherwise
        error('Mex function "gForce_mex" not found. Check that it is compiled and added to the path.')
end

%% Main functionality
% connecting the device.
% NOTE: Any gForce command starts the transmission from the device.

% -- Getting battery
bat = gForce_mex('getBattery');
fprintf('\n\nDevice battery is %d\n\n', bat)

% -- Verbose
gForce_mex('verbose', 0);


% -- Vibrating, ms
gForce_mex('vibrate', 300);

% -- getting Emg
data = gForce_mex('getEmg');
if ~isempty(data)
    fprintf('\n\nEmg data size: (%d, %d)\n\n', size(data))
    plot(data(3, :)),title('Emg: channel 3')
else
    warning('Emg data not retrieved')
end

% -- Quaternions
gForce_mex('enableQuaternions', 1);
pause(1) % wait for data

dataQ = gForce_mex('getQuaternions');
if ~isempty(dataQ)
    fprintf('\n\nQuaternions data size: (%d, %d)\n\n', size(dataQ))
    figure,plot(dataQ'),title('Quaternions'),legend('w','x','y','z')
else
    warning('Quaternions data not retrieved')
end

% -- changing Emg resolution
wasSet = gForce_mex('setEmgResolution', 12);
if wasSet
    fprintf('\n\nEmg ADC resolution correctly set at 12 bits.\n\n')
else
    warning('\n\nResolution could not be set at 12 bits.\n\n')
end

%% More information
% There is several options available at the documentation.
% Run the help command.

% help gForce_mex