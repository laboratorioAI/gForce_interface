%gForce_mex is a C++ MEX function built with Visual Studio. This function
%is the interface between Matlab and the GForce-Pro armband by OyMotion. It
%can fetch Emg, orientation and gesture data from the armband.
% Default Emg configuration is 8 bits ADC at 500Hz.
% Orientation data is represented only with quaternions. Quaternions
% convention is w,x,y,z at 50Hz. By default quaternions are enabled.
% The whole functionality is described through the following commands.
% Please review them deeply.
%
% # METHODS
%   ## Emg
%       gForce_mex('getEmg') returns an 8-by-m matrix with the Emg data
%       accumulated since the last call of 'getEmg', 'clearEmg' or the
%       first call of gForce_mex. In the case ADC resolution is 8 bits, the
%       returned value is (uint8), on the contrary, a resolution of 12 bits
%       returns a (uint16). The signal is centered in 127 when 8 bit
%       resolution or in 2048 at 12 bits resolution.
%       gForce_mex('clearEmg') clears the buffer of the Emg signal. This
%       process is run everytime the frequency or the resolution is
%       changed, so that different frequency data is not mixed.
%       gForce_mex('setEmgResolution', bitResolution) sets the resolution
%       of the ADC converter whether to 8 or 12 bits. Returns true when
%       everything went ok. This configuration changes the type of array
%       returned by 'getEmg'. Default is 8 bits. This method clears the Emg
%       buffer.
%       Frequency ranges by resolution:
%           8 bits resolution maximum 1000Hz,
%           12 bits resolution maximum 500Hz. In the case frequency was in
%           a higher value than 500Hz, it is changed automatically.
%       gForce_mex('setEmgSamplingRate', frequency) sets the sampling rate
%       of the Emg ADC converter. View Frequency ranges by resolution.
%       Returns true when everything went ok. This method clears the Emg
%       buffer.
%   ## Orientation
%       gForce_mex('getQuaternions') returns a 4-by-m matrix. Be aware, the
%       quaternions have convention [w,x,y,z].
%       gForce_mex('clearQuaternions') clears the buffer of quaternions.
%       gForce_mex('enableQuaternions', enableFlag) enables or disables the
%       quaternions based on enableFlag (0, 1). By default, quaternions are
%       enabled.
%   ## Gestures
%       ...Currently unavailable...
%       [classes, timestamp, time_ref] = gForce_mex('getPredictions')
%       returns 3 values from the HGR model embedded in the gForce-Pro.
%           classes is a n-by-1 vector (string) with the name of the
%           predicted classes on every event prediction.
%           timestamp is a n-by-1 vector (double) of the time in ms elapsed
%           since time_ref (the time reference) of every prediction.
%           time_ref is a struct with the time reference. This struct has
%           the fields: sec, min, hour, day, mon, year.
%           This time reference is the moment from where the timestamp
%           vector is counted. The time reference is initializated in the
%           connection of the device. It is restarted on every call to
%           'clearPredictions'.
%       Important: the gForce SDK raises an event on every predictions.
%       This means that if no gesture was predicted, the classes and
%       timestamp will be empty.
%       gForce_mex('clearPredictions') clears the buffer of the predictions.
%   ## Auxiliar
%       gForce_mex('getBattery') returns (int8) the energy percentage. In
%       case something went wrong returns -1.
%       gForce_mex('vibrate', timeMS) vibrates the device timeMS ms.
%       gForce_mex('powerOff') powers off the device!
%       gForce_mex('verbose', flag) when flag equalst 1 enables C++ MEx
%       function to display (a lot of) messages in the command window.
%       In the case is 0, it does not display anything.
%       gForce_mex('getStatus') returns true when the device is connected
%       correctly.
%
% # NOTES
%       1. All gForce commands start the connection and trasmission of the
%       armband. So buffers (i.e. Emg, orientation, gesture prediction) are
%       filled since the very beginning.
%       2. All gForce commands must be char vectors, for example:
%           data = gForce_mex('getEmg')  % correct
%           % data = gForce_mex("getEmg")  % error!
%       3. IMU data is not accesible by the "feature map issue". Only the
%       orientation data can be fetched. Check the Extended documentation
%       for more info.
%       3. GForce-Pro can not be pause (i.e. always is transmitting data),
%       so it is recommended to turn it off manually. This is not an
%       inconvenient, for the connection time is insignificant.
%       4. Most invalid commands, although raise an error do not crash
%       matlab.
%       5. The SDK generates two auxiliary files in the current directory
%       in every connection to the device. One of the files is a .txt log
%       file that can only be erased when the dongle is disconnected or
%       matlab restarted.
%
% # KNOWN ISSUES
%       1. In the case of a Run Time Error in the C++ MEX function, Matlab
%       will crash inmediately and close itself. The device must be
%       restarted manually.
%       2. Battery level sometimes returns 0. This is the response from the
%       device (the SDK not the C++ MEX function). It may not correspond to
%       a completely empty battery, as in some tests afterwards the command
%       returned a value (e.g. 77).
%       3. Temperature requests always returns 0, it is buggy from the SDK.
%       4. Emg data at 12 bit resolution is not centered at 2047 (as it
%       must be), instead it is centered in a value slightly lower than
%       2000. Something similar happens at 8 bit resolution.
%       5. Gesture predictions are currently unavailable due to empty
%       responses from the device.
%
% # EXAMPLES
% Retrieving the emg data:
%   >> data = gForce_mex('getEmg');
%   >> size(data)
% [labIA][06-Jan-2021 23:01:34] Connecting
% (2021-1-6 23:01:34:165)[I/gForceSDK]: setWorkMode. 1
% (2021-1-6 23:01:34:165)[I/gForceSDK]: Please call method run(ms) to pull
% message.
% [labIA][06-Jan-2021 23:01:34] Registering Listener...
% [labIA][06-Jan-2021 23:01:34] Listener registered
% [labIA][06-Jan-2021 23:01:34] Registering Hub...
% [labIA][06-Jan-2021 23:01:35] Hub registered
% [labIA][06-Jan-2021 23:01:35] Scanning devices...
% [labIA][06-Jan-2021 23:01:35] Device found
% [labIA][06-Jan-2021 23:01:35] run!
% [labIA][06-Jan-2021 23:01:38] Return code of running was error: time out
% [labIA][06-Jan-2021 23:01:38] Finished run!
% [labIA][06-Jan-2021 23:01:38] Checking matlab command
% [labIA][06-Jan-2021 23:01:38] Input command seems fine!
% [labIA][06-Jan-2021 23:01:38] getEmg
% [labIA][06-Jan-2021 23:01:38] Checking unique input
% [labIA][06-Jan-2021 23:01:38] Configuring output!
% [labIA][06-Jan-2021 23:01:38] Emg construction!
% [labIA][06-Jan-2021 23:01:38] Emg returned successfully!
%
% ans =
%
%      8   1120
%
%
% Hiding the annoying msjs of gForce_mex and continuing with life:
%   >> gForce_mex('verbose', 0)
%   >> data = gForce_mex('getEmg');
%   >> size(data)
% [labIA][06-Jan-2021 23:08:05] Checking matlab command
% [labIA][06-Jan-2021 23:08:05] Input command seems fine!
% [labIA][06-Jan-2021 23:08:05] verbose
% [labIA][06-Jan-2021 23:08:05] Checking auxiliar input
%
% ans =
%
%            8       25648
%
%
% Retrieving orientation data:
%   >> gForce_mex('enableQuaternions', 1);
%   >> pause(1)
%   >> data = gForce_mex('getQuaternions');
%   >> plot(data')
%
%
% Getting battery:
% >> gForce_mex('getBattery')
% [labIA][06-Jan-2021 23:06:08] Checking matlab command
% [labIA][06-Jan-2021 23:06:08] Input command seems fine!
% [labIA][06-Jan-2021 23:06:08] getBattery
% [labIA][06-Jan-2021 23:06:08] Checking unique input
% [labIA][06-Jan-2021 23:06:08] Asking battery level.
% [labIA][06-Jan-2021 23:06:08] Result of async func bat is: success
% [labIA][06-Jan-2021 23:06:08] Waiting battery.
% [labIA][06-Jan-2021 23:06:08] Battery state is: success
%
% ans =
%
%   int8
%
%    74
%
%
% # INSTALLATION
% The installation of the GForce-MEX utility is a very fateful, challenging
% yet unavoidable process (as it depends on the operating system and matlab
% version). Please follow the Installation Guide carefully.
% In the Installation you will find some interesing details about the
% compilation of this C++ MEX function.

%{
Laboratorio de Inteligencia y Visión Artificial
ESCUELA POLITÉCNICA NACIONAL
Quito - Ecuador

Quejas, reclamos o donaciones a:
autor: ztjona!
jonathan.a.zea@ieee.org

"I find that I don't understand things unless I try to program them."
-Donald E. Knuth

31 December 2020
Matlab R2020b.
%}
