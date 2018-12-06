close all
clear all
load('ber.dat');
x=[0:1:12];
x_linear=10.^(x/10);
BER=0.5*erfc(sqrt(x_linear));

semilogy(x,BER)
hold on
x=[0:1:length(ber)-1];
semilogy(x,ber,'ro-.')
legend('Theory','SystemC-AMS')
xlabel('Eb/N0 (dB)')
ylabel('BER')
grid on
hold off

figure
receiver2=load('receiver2.dat');
plot(receiver2(:,2),receiver2(:,3),'o');
grid on
