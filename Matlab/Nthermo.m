close all;
clear all;
format long;
scale=1024/5;
amplification=500;

Vn=scale*amplification/1000*[0, 0.659, 1.065, 2.189,3.989, 4.302, 4.618, 4.937, 5.259, 5.585,5.913,6.245, 6.579,...
    6.916, 7.255, 7.597,7.941, 8.288, 8.637, 8.988,9.341, 9.696,10.054,10.413,10.774, 11.136,11.501, 11.867, 12.234, 12.603, 12.974, 13.346, 13.719, 14.094, 14.469, 14.848, 15.225, 15.604, 15.984, 16.366, 16.748];
Tn=[0,25,40,80,140,150,160,170,180,190,200,210,220,...
    230,240,250,260,270,280,290,300,310,320,330,340,350,360,370,380,390,400,410,420,430,440,450,460,470,480,490,500 ];

plot(Tn,Vn);

for i=2:2:82
  gah(i)=Tn(floor(i/2));   
end    

for i=1:2:81
  gah(i)=Vn(floor((i+1)/2));   
end 


gah=round(gah);



maximum= 1;
for j=1:size(Vn,2)
       
    lollotiloo=strcat('{',num2str(round(Vn(j))),',',num2str(Tn(j)) ,'},');
    if size(lollotiloo,2) > maximum
       maximum=  size(lollotiloo,2);
    end
    
    for i=1:maximum
        test2(j,i)=lollotiloo(i)
    end
     
end

dlmwrite('ntype.txt',test2,'delimiter','','newline','pc');



