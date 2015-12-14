#!/usr/bin/env sh

gnuplot create_plot.plt
inkscape --without-gui -f ukrytych_od_podstawowych.svg -E ../images/ukrytych_od_podstawowych.eps
