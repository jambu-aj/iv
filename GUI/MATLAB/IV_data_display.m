%IV Data Display 512 Bytes
function IV = IV_data_display(datafile);
dec=zeros(256);
res=zeros(256);
f=64; %sample frequency Hz
sample=1:256;
time=sample/f;
t=1;
IVID = fopen(datafile,'r');
Header = textscan(IVID,'%s',2,'Delimiter','\n');
C = textscan(IVID, '%s', 'Delimiter', '');
C = regexp(C{:}, '\w+', 'match');
lengthofdata = length(C);
for i=1:lengthofdata
    for j=3:2:17
        hex_string=strcat(C{i}{j},C{i}{j-1});
        dec(t)=hex2dec(hex_string);
        res(t)=dec(t)*0.154957;
        t=t+1;
    end
end
str = sprintf('SD CARD Bioimpedance data: %s',datafile);
plot(time,res)
title(str)
ylabel('Resistance (Ohm)')
xlabel('Time (Sec)')

fclose(IVID);
end