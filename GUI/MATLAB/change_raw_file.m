filename = 'HW1256.TXT';

%urlname = ['file:///' fullfile(pwd,filename)];
%try
 %   str = urlread(urlname);
%catch err
 %   disp(err.message)
%end

%filename = 'myfile02.txt';
[A,delimiterOut]=importdata(filename)