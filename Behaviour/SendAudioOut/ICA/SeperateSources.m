function [outFrame] = SeperateSources( inFrame,angle )
%SEPERATESOURCES use ICA to seperate sources

%use EEGLab's runica() to do ICA
%[~,~,~,~,~,~,outFrame] = runica(inFrame,'verbose','off');

%using FastICA algorithm from http://research.ics.aalto.fi/ica/fastica/code/dlcode.shtml
[outFrame]=fastica(inFrame,'verbose','off');

%outFrame=inFrame;

end

