format long;
clear all;

A=((20E6)./[8,64,256,1024]);
B=1:(2^8);

K=(B./A(3))*1000;
%K=1./K
K1=(B./A(3));
K1=1./K1
ga=floor(K);
fract=K-ga;
%next finds the zero elements which are in ms
exact=find(~fract)

ga=floor(K1);
fract=K1-ga;
%next finds the zero elements which are in ms
exact=find(~fract)


