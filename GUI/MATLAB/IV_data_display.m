%IV Data Display 512 Bytes
function IV = IV_data_display(datafile);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
f=64; %sample frequency Hz
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%4 modalities (RTD1, RTD2, BioZ, Strain)
n=4;
f_n=f/n; 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
        t=t+1;
    end
end

lod = length(dec);
ind=1:lod;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%      Temperature data        %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%RTD1
ind1=mod(ind,n)==1;
Temp1_code=dec(ind1);
s1=length(Temp1_code);
time_Temp1=1:s1;
time_Temp1=time_Temp1/f_n;
Temp1 = 3400 - 869565*sqrt(1.57042*10^(-5)-(1.24047*10^(-9)*Temp1_code));

%RTD2
ind2=mod(ind,n)==2;
Temp2_code=dec(ind2);
s2=length(Temp2_code);
time_Temp2=1:s2;
time_Temp2=time_Temp2/f_n;
Temp2 = 3400 - 869565*sqrt(1.57042*10^(-5)-(1.24047*10^(-9)*Temp2_code));

%Difference between RTD1 and RTD2
if(s1>s2)
    Temp1=Temp1(1:end-1);
end
Temp_Abs = abs(Temp1-Temp2);
time_Abs=time_Temp2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%      BioImpedence data       %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

ind3=mod(ind,n)==3;
BioZ_code=dec(ind3);
time_BioZ=1:length(BioZ_code);
time_BioZ=time_BioZ/f_n;
BioZ = BioZ_code*0.154957;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%         Strain data          %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

ind4=mod(ind,n)==0;
Strain_code=dec(ind4);
time_Strain=1:length(Strain_code);
time_Strain=time_Strain/f_n;
Strain = (Strain_code*1.7)/(2^15);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%         Plotting             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Figure 1 - Temperature

figure(1)
subplot(2,2,1)
plot(time_Abs,Temp1)
title('SD CARD data: RTD1')
ylabel('Degree Celcius')
xlabel('Time (Sec)')

subplot(2,2,2)
plot(time_Temp2,Temp2)
title('SD CARD data: RTD2')
ylabel('Degree Celcius')
xlabel('Time (Sec)')

subplot(2,2,[3:4])
plot(time_Temp2,Temp_Abs)
title('SD CARD data: |RTD1-RTD2|')
ylabel('Degree Celcius')
xlabel('Time (Sec)')

%Figure 2 - Bioimpedance
figure(2)
%subplot(2,2,3)
plot(time_BioZ,BioZ)
title('SD CARD data: Bioimpedance')
ylabel('Resistance (ohm)')
xlabel('Time (Sec)')

%Figure 3 - Strain
figure(3)
%subplot(2,2,4)
plot(time_Strain,Strain)
title('SD CARD data: Strain')
ylabel('Strain')
xlabel('Time (Sec)')

fclose(IVID);
end