classdef GForce
    %GForce is a class that handles the connection of the GForce pro
    %device. This class just encapsulates the functionallyty of gForce_mex
    %calling its methods with predefined configurations (hence, it requires
    %a compiled version of gForce_mex). View more details about gForce_mex.
    
    %{
    Laboratorio de Inteligencia y Visión Artificial
    ESCUELA POLITÉCNICA NACIONAL
    Quito - Ecuador
    
    autor: ztjona!
    jonathan.a.zea@ieee.org
    Cuando escribí este código, solo dios y yo sabíamos como funcionaba.
    Ahora solo lo sabe dios.
    
    "I find that I don't understand things unless I try to program them."
    -Donald E. Knuth
    
    20 January 2021
    Matlab 9.9.0.1538559 (R2020b) Update 3.
    %}
    
    %%
    properties (SetAccess = private)
        % flag
        isConnected = false;
        
        % 8 or 12 bits
        emgResolution = 8;
        
        % Hz
        emgFreq = 500;
        
        %
        enabledQuats = 1; % 1 or 0
        
        % enabledPredictions = false; % future (unavailable)
        
        verbose = 0;
    end
    
    properties (Constant)
        
    end
    
    methods
        %% Constructor
        % -----------------------------------------------------------------
        function obj = GForce(options)
            % Constructor: THe device is configured depending on the fields
            % of options related to different parameters of the GForce pro
            % device.
            %
            %
            %# Inputs
            %* profile   -struct with fields. Check config_GForce auxiliary
            %   function for details.
            %
            %# Outputs
            %* obj     -objeto GForce
            
            %             % ---------- Data Validation
            %             arguments
            %                 profile (1, :) char
            %             end
            %
            %             % # ----- loading profile
            %             options = config_GForce(profile);
            
            
            % # ----- getting configurations
            if isfield(options, 'emgResolution')
                obj.emgResolution = options.emgResolution;
            end
            
            if isfield(options, 'emgFreq')
                obj.emgFreq = options.emgFreq;
            end
            
            if isfield(options, 'verbose')
                obj.verbose = options.verbose;
            end
            
            if isfield(options, 'enabledQuats')
                 obj.enabledQuats = double(options.enabledQuats);
            end
            
            % # ----- config device
            gForce_mex('verbose', obj.verbose);
            
            obj.isConnected = gForce_mex('getStatus');
            if ~obj.isConnected
                warning('Not connected')
                return;
            end
            if ~gForce_mex('setEmgResolution', obj.emgResolution)
                error('resolution could not be set')
            end
            
            if ~gForce_mex('setEmgSamplingRate', obj.emgFreq)
                error('Frequency could not be set')
            end
            
            if ~gForce_mex('enableQuaternions', obj.enabledQuats)
                error('Could not enabled/disabled quaternions')
            end
        end
        
        
        %%
        % -----------------------------------------------------------------
        function obj = clear(obj)
            %obj.clear clears the Emg data and Quaternions, if enabled.
            %
            % # Example
            %    = obj.clear()
            %
            
            % # ----
            gForce_mex('clearEmg');
            
            if obj.enabledQuats
                gForce_mex('clearQuaternions');
            end
            
            % not defined
            %if obj.enabledPredictions
            %   gForce_mex('clearPredictions');
            %end
        end
        
        % %% Destructor
        % % -----------------------------------------------------------------
        % function delete(obj)
        %     %obj.delete() just power offs the device
        %     %
        %
        %     % # ----
        % % gForce_mex('powerOff');
        % end
        %
        %%
        % -----------------------------------------------------------------
        function data = getEmg(obj)
            %obj.getEmg returns collected emg data.
            %
            %# Inputs
            %
            %# Outputs
            %* data         -(8, m) uint
            %
            % # Example
            %   data = obj.getEmg()
            %
            
            % # ----
            if obj.isConnected
                data = gForce_mex('getEmg');
            else
                error('device not connected')
            end
        end
        
        
        %%
        % -----------------------------------------------------------------
        function data = getOrientation(obj)
            %obj.getOrientation returns collected quaterions data if
            %enabled, otherwise raises error.
            %
            %# Inputs
            %
            %# Outputs
            %* data         -(4, m) double
            %
            % # Example
            %   data = obj.getOrientation()
            %
            
            % # ----
            if obj.isConnected && obj.enabledQuats
                data = gForce_mex('getQuaternions');
            else
                error('device not connected nor are quaternions enabled')
            end
        end
        
        %%
        % -----------------------------------------------------------------
        function data = getPredictions(obj)
            %obj.getPredictions returns collected gestures predicted.
            %
            %# Inputs
            %
            %# Outputs
            %* data         -(4, m) double
            %
            % # Example
            %   data = obj.getOrientation()
            %
            
            % # ----
            if obj.isConnected && obj.enabledPredictions
                data = gForce_mex('getPredictions');
            else
                error('device not connected nor are predictions enabled')
            end
        end
        %%
        % -----------------------------------------------------------------
        function data = getBattery(obj)
            %obj.getBattery returns the battery.
            %
            
            % # ----
            if obj.isConnected
                data = gForce_mex('getBattery');
            else
                error('device not connected and could not retrieve battery')
            end
        end
        
        %%
        % -----------------------------------------------------------------
        function vibrate(obj, msecs)
            %obj.vibrate sends the command to vibrate the device for an
            %amount of ms.
            %
            %# Inputs
            %* msecs        -double in ms
            %
            % # ---- Data Validation
            arguments
                obj
                msecs (1,1) double {mustBePositive} = 100;
            end
            
            % # ----
            if obj.isConnected
                gForce_mex('vibrate', msecs);
            else
                error('Device not connected')
            end
        end
    end
end