close all
clear all
load fft_sc.dat
stairs(fft_sc(:,1),db(fft_sc(:,2)))