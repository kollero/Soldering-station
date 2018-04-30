close all;
clear all;
format long;
scale=1024/5;
amplification=500;

Vc=scale*amplification/1000*[0,0.067,0.342,0.555,1.145,2.087,2.415,2.75,3.09,3.436,3.786,...
    4.141,4.501,4.865,5.232,5.417,5.603,5.789,5.976,6.164,6.353,6.542,...
    6.732,6.922,7.113,7.497,7.882,8.269,8.657];
Tc=[0,5,25,40,80,140,160,180,200,220,240,...
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

dlmwrite('ctype.txt',test2,'delimiter','','newline','pc');
