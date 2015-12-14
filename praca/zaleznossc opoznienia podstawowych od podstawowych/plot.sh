#!/usr/bin/env sh

gnuplot create_plot.plt
inkscape --without-gui -f podstawowych_od_podstawowych.svg -E ../images/podstawowych_od_podstawowych.eps
