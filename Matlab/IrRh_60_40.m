
close all;
clear all;
format long;
scale=1024/5;
amplification=500;

Vc=scale*amplification/1000*[0,0.135,0.29,0.55,0.84,1.05,1.27,1.38,1.49,...
    1.61,1.725,1.845,1.96,2.08,2.32,2.564,2.87, 3.17,3.775,4.94];

Tc=[25,40,80,140,200,240,280,300,320,...
    340,360,380,400,420,460,500,550,600,700,900];





%input offset voltage drift by temp
%Vc=Vc+25*50E-9;

plot(Tc,Vc);

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

dlmwrite('IrRh60_40.txt',test2,'delimiter','','newline','pc');