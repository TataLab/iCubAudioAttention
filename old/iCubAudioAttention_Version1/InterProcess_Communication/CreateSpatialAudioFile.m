function [ok] = CreateSpatialAudioFile(fileName,L)
%build a new file, pre-fill with zeros, return OK if it worked

try
audioD = single(zeros(2,L)); %one channel because the output signals of the audio modules are "cyclopean" plus a vector of time stamps

fileID=fopen(fileName,'w');
fwrite(fileID,audioD,'single');
fclose(fileID);
ok=1;

catch %#ok to not catch the error in a fancy way
    ok = 0;
end

return;