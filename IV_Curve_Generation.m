% PCB Design IV Curve Generation Script
clear all; close all; clc;

% arduino is on COM3, use 115200 baud
% call delete(arduino) when finished
arduino = serialport("COM3", 115200);
configureTerminator(arduino, "CR/LF");
arduino.Timeout = 10;
arduino.UserData = struct(      ...
    "Count", [],                ...
    "I", [],                    ...
    "V", [],                    ...
    "Rsense_Ohms", 1000,        ...
    "Expected_Lines", 512       ...
    );

readline(arduino);
% initiate processing
writeline(arduino, "START");
% start callback
configureCallback(arduino,"terminator", @readData);

function floatVal = ADC_RawToFloat(raw)
    % ADC settings
    ADC_NBITS = 16;     % 16-bit
    ADC_FSR   = 6.144;  % +/- full scale range
    floatVal = raw/2^(ADC_NBITS-1)*ADC_FSR;
end

function readData(src, ~)
    
    C = strsplit(readline(src), ',');
    if size(C,2)==3
        src.UserData.Count(end+1) = str2double(C(1));
        src.UserData.I(end+1) = ADC_RawToFloat(str2double(C(2)))...
                                /src.UserData.Rsense_Ohms*1000;
        src.UserData.V(end+1) = ADC_RawToFloat(str2double(C(3)));
    end
    
    if size(src.UserData.Count, 2) >= src.UserData.Expected_Lines
        configureCallback(src, "off");
        plot(src.UserData.V, src.UserData.I, '-b', 'LineWidth', 1);
        %xlim([0 5]);
        %ylim([0 5]);
        grid on;
        xlabel('Voltage (V)');
        ylabel('Current (mA)');
    end
end