close all;
clear all;
format long;
scale=1024/5;
amplification=500;

Vc=scale*amplification/1000*[0,0.397,1.0,1.612,3.267,5.735,6.54,7.34,8.138,8.94,9.747,...
    10.561,11.382,12.209,13.040,13.457,13.874,14.293,14.713,15.133,15.554,15.975,...
    16.397,16.82,17.243,18.081,18.941,19.792,20.644];
Tc=[0,10,25,40,80,140,160,180,200,220,240,...
    260,280,300,320,330,340,350,360,370,380,390,...
    400,410,420,440,460,480,500];

%input offset voltage drift by temp
%Vc=Vc+25*50E-9;

plot(Tc,Vc);

for i=2:2:length(Vc)*2
  gah(i)=Tc(floor(i/2));   
end    

for i=1:2:(length(Vc)*2)-1
  gah(i)=Vc(floor((i+1)/2));   
end 


gah=round(gah);



maximum= 1;
for j=1:size(Vc,2)
       
    lollotiloo=strcat('{',num2str(round(Vc(j))),',',num2str(Tc(j)) ,'},');
    if size(lollotiloo,2) > maximum
       maximum=  size(lollotiloo,2);
    end
    
    for i=1:maximum
        test2(j,i)=lollotiloo(i)
    end
     
end

dlmwrite('ktype.txt',test2,'delimiter','','newline','pc');
