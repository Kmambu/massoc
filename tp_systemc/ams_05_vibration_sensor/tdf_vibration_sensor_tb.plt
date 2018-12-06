###############################################################################
# Plot transient simulation results of the vibration sensor model.
###############################################################################
# File:    tdf_vibration_sensor_tb.plt
# Author:  Torsten Maehne
# Version: 1.0
###############################################################################

###############################################################################
# DAT-File: tdf_vibration_sensor_tb.dat
# 1: time
# 2: x_sig
# 3: v_sig
# 4: frontend.v_amp_sig
# 5: frontend.amp_sig
# 6: adc_sig
# 7: k_sig
###############################################################################

reset
set terminal postscript eps enhanced color size 7,4
#set terminal postscript eps enhanced mono
set output 'tdf_vibration_sensor_tb.eps'

set key outside samplen 2 spacing 2.5 width 0 box
set grid
set origin 0.0, 0.0
set lmargin 8
set rmargin 12

#set xrange [0.0:12.0]
set xrange [10.0:22.0]
set xtics 1
set mxtics 2

set format y "%4.4g"

set multiplot

# Vibration displacement
set size 1.0, 0.16
set origin 0.0, 0.84
set key outside spacing 2.3 width 1 box

set xlabel ""
set format x ""

set ylabel "x / {/Symbol m}m"
# set yrange [-60:60]
# set ytics 30
set yrange [-14:-2]
set ytics 4
#set mytics 1

plot 'tdf_vibration_sensor_tb.dat' using (1e3*$1):(1e6*$2) title "x" with lines

# Sensor output signal
set size 1.0, 0.16
set origin 0.0, 0.68
set key outside spacing 2.5 width 1.5 box

set xlabel ""
set format x ""

set ylabel "v / mV"
# set yrange [-4:4]
# set ytics 2
#set mytics 2
set yrange [-280:280]
set ytics 140

plot 'tdf_vibration_sensor_tb.dat' using (1e3*$1):(1e3*$3) title "v" with lines

# Amplified sensor signal
set size 1.0, 0.16
set origin 0.0, 0.52
set key outside spacing 2.5 width 1.5 box

set xlabel ""
set format x ""

set ylabel "v_{amp} / V"
set yrange [-6:6]
set ytics 5
#set mytics 2

plot 'tdf_vibration_sensor_tb.dat' using (1e3*$1):4 title "v_{amp}" with lines

# ADC output signal
set size 1.0, 0.16
set origin 0.0, 0.36
set key width 1.5

set xlabel ""
set format x ""

set ylabel "adc / 1"
# set yrange [-5:5]
# set ytics 2
set yrange [-16:16]
set ytics 8
#set mytics 2

plot 'tdf_vibration_sensor_tb.dat' using (1e3*$1):6 title "adc" with lines

# Average amplitude signal
set size 1.0, 0.16
set origin 0.0, 0.20
set key width 1.0

set xlabel ""
set format x ""

set ylabel "amp / 1"
#set yrange [-2:18]
set yrange [0:12]
#set ytics 5
set ytics 4
#set mytics 2

plot 'tdf_vibration_sensor_tb.dat' using (1e3*$1):5 title "amp" with lines

# Gain output signal
set size 1.0, 0.20
set origin 0.0, 0.0
set key width 0.5

set xlabel "t / ms"
set format x "%g"

# set ylabel "gain / 10^3"
# set yrange [0:35]
# set ytics 10
# #set mytics 2

# plot 'tdf_vibration_sensor_tb.dat' using (1e3*$1):(1e-3*$23) title "gain" with lines

set ylabel "2^k / 1"
set yrange [-0:136]
set ytics 32
#set mytics 2

plot 'tdf_vibration_sensor_tb.dat' using (1e3*$1):(2**$7) title "gain" with lines

unset multiplot
