close all;
file_transmitter1=load('transmitter1.dat');
file_transmitter2=load('transmitter2.dat');
file_transmitter3=load('transmitter3.dat');
file_receiver1=load('receiver1.dat');
file_receiver2=load('receiver2.dat');
file_receiver3=load('receiver3.dat');

time1=file_transmitter1(:,1);
pulse=file_transmitter1(:,2);
encoder=file_transmitter1(:,3);

time2=file_transmitter2(:,1);
demuxI=file_transmitter2(:,2);
demuxQ=file_transmitter2(:,3);

time3=file_transmitter3(:,1);
modulatorI=file_transmitter3(:,2);
modulatorQ=file_transmitter3(:,3);
adder=file_transmitter3(:,4);
transmitter=file_transmitter3(:,5);

channel=file_receiver1(:,2);
lna=file_receiver1(:,3);
demodulatorI=file_receiver1(:,4);
demodulatorQ=file_receiver1(:,5);
integratorI=file_receiver1(:,6);
integratorQ=file_receiver1(:,7);

samplerI=file_receiver2(:,2);
samplerQ=file_receiver2(:,3);
decisionI=file_receiver2(:,4);
decisionQ=file_receiver2(:,5);

receiver=file_receiver3(:,2);

figure;
subplot(2,3,1)
stairs(time1,pulse);
title('pulse','FontSize',20);
ylim([-0.5,1.5]);
subplot(2,3,2)
stairs(time1,encoder);
title('encoder','FontSize',20);
ylim([-2,2]);
subplot(2,3,3)
stairs(time2,demuxI);
title('demuxI','FontSize',20);
ylim([-2,2]);
subplot(2,3,6)
stairs(time2,demuxQ);
title('demuxQ','FontSize',20);
ylim([-2,2]);

figure;
subplot(2,3,1)
plot(time3,modulatorI);
title('modulatorI','FontSize',20);
subplot(2,3,4)
plot(time3,modulatorQ);
title('modulatorQ','FontSize',20);
subplot(2,3,2)
plot(time3,adder);
title('adder','FontSize',20);
subplot(2,3,3)
plot(time3,transmitter);
title('PA','FontSize',20);

figure;
subplot(2,3,1)
plot(time3,channel);
title('channel','FontSize',20);
subplot(2,3,2)
plot(time3,lna);
title('LNA','FontSize',20);
subplot(2,3,3)
plot(time3,demodulatorI);
title('demodulatorI','FontSize',20);
subplot(2,3,6)
plot(time3,demodulatorQ);
title('demodulatorQ','FontSize',20);

figure;
subplot(2,4,1)
plot(time3,integratorI);
title('integratorI','FontSize',20);
subplot(2,4,5)
plot(time3,integratorQ);
title('integratorQ','FontSize',20);
subplot(2,4,2)
stairs(time2,samplerI);
title('samplerI','FontSize',20);
ylim([-2,2]);
subplot(2,4,6)
stairs(time2,samplerQ);
title('samplerQ','FontSize',20);
ylim([-2,2]);
subplot(2,4,3)
stairs(time2,decisionI);
title('decisionI','FontSize',20);
ylim([-2,2]);
subplot(2,4,7)
stairs(time2,decisionQ);
title('decisionQ','FontSize',20);
ylim([-2,2]);
subplot(2,4,4)
stairs(time1,receiver);
title('mux','FontSize',20);
ylim([-2,2]);

figure;
plot(samplerI,samplerQ,'o');
title('constellation','FontSize',20);
axis([-2,2,-2,2]);
grid on
