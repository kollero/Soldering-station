close all;
clear all;
format long;
scale=1024/5;
amplification=500;

%Tc1000=[25,63, 80,120,146,182,212,242,267,300,380];
%ADC1000=[1111,102,136,229,296,388,475,566,666,762,1000];
  

Tc=[32,46,85, 130,202,230,262,294,330,350,400,430,470,486,514];    

ADC=[0,6,30,53, 83,99,116,133,150,160,200,219,230, 245,250];

%plot(Tc,ADC);
%figure;

Tc=round(smooth(Tc)');
ADC=round(smooth(ADC)');
 % Tc=ctranspose(Tc);

plot(Tc,ADC);

maximum= 1;
for j=1:size(ADC,2)
       
    lollotiloo=strcat('{',num2str(round(ADC(j))),',',num2str(Tc(j)) ,'},')
    if size(lollotiloo,2) > maximum
       maximum= size(lollotiloo,2)
    end
    
    for i=1:maximum
        test2(j,i)=lollotiloo(i);
    end
     
end

dlmwrite('ownthermo.txt',test2,'delimiter','','newline','pc');



