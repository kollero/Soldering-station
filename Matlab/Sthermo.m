close all;
clear all;
format long;
scale=1024/5;
amplification=500;

Vc=scale*amplification/1000*[0,0.055,0.143,0.235,0.502,0.95,1.11,1.273,1.441,1.612,1.786,...
    1.962,2.141,2.323,2.507,2.599,2.692,2.786,2.88,2.974,3.069,3.164,...
    3.259,3.355,3.451,3.645,3.84,4.036,4.233];

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
       maximum=  size(lollotiloo,2)
    end
    
    for i=1:maximum
        test2(j,i)=lollotiloo(i);
    end
     
end

dlmwrite('Stype.txt',test2,'delimiter','','newline','pc');
