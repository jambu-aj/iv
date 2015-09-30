%IV Data Display 512 Bytes
function IV = IV_data_display(datafile);
%dec=zeros(256);
%res=zeros(256);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
f=64; %sample frequency Hz
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
f_3=f/3; %3 modalities
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
       % res(t)=dec(t)*0.154957;
        t=t+1;
    end
end
lod = length(dec);
ind=1:lod;

%Temperature data
ind1=mod(ind,3)==1;
Temp_code=dec(ind1);
time_Temp=1:length(Temp_code);
time_Temp=time_Temp/f_3;
Temp = 3400 - 869565*sqrt(1.57042*10^(-5)-(1.24047*10^(-9)*Temp_code));

%BioImpedence data
ind2=mod(ind,3)==2;
BioZ_code=dec(ind2);
time_BioZ=1:length(BioZ_code);
time_BioZ=time_BioZ/f_3;
BioZ = BioZ_code*0.154957;

%Strain data
ind3=mod(ind,3)==0;
Strain_code=dec(ind3);
time_Strain=1:length(Strain_code);
time_Strain=time_Strain/f_3;
Strain = (Strain_code*1.7)/(2^15);

%Plotting
%str = sprintf('SD CARD data: %s',datafile);
subplot(3,1,1)
plot(time_Temp,Temp)
title('SD CARD data: Temperature')
ylabel('Degree Celcius')
xlabel('Time (Sec)')

subplot(3,1,2)
plot(time_BioZ,BioZ)
title('SD CARD data: Bioimpedance')
ylabel('Resistance')
xlabel('Time (Sec)')

subplot(3,1,3)
plot(time_Strain,Strain)
title('SD CARD data: Strain')
ylabel('Strain')
xlabel('Time (Sec)')

fclose(IVID);
end