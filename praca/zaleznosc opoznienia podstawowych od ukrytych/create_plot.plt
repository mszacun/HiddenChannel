set terminal svg enhanced font 'Verdana,8'
set output './podstawowych_od_ukrytych.svg'

set title "Zależność średniego opóźnienia wiadomości podstawowych\n od intensywności napływu wiadomości ukrytych"
set xlabel 'Intensywność napływu wiadomości ukrytych'
set ylabel 'Średnie opóźnienie wiadomości podstawowych'

plot 1/0 with point lt rgb "#0072bd" pt 5  lw 3 title "Intensywność napływu wiadomości podstawowych = 1, średnia długość wiadomości podstawowej = 100 bajtów", "natezenie_1_dlugosc_100.csv" lt rgb "#0072bd" pt 5 ps 0.3 notitle, \
    1/0 with point lt rgb "#d95319" pt 7  lw 3 title "Intensywność napływu wiadomości podstawowych = 5, średnia długość wiadomości podstawowej = 100 bajtów", "natezenie_5_dlugosc_100.csv" lt rgb "#d95319" pt 7 ps 0.3 notitle, \
    1/0 with point lt rgb "#edb120" pt 9  lw 3 title "Intensywność napływu wiadomości podstawowych = 9, średnia długość wiadomości podstawowej = 100 bajtów", "natezenie_9_dlugosc_100.csv" lt rgb "#edb120" pt 9 ps 0.3 notitle, \
    1/0 with point lt rgb "#7e2f8e" pt 11 lw 3 title "Intensywność napływu wiadomości podstawowych = 5, średnia długość wiadomości podstawowej = 30 bajtów", "natezenie_5_dlugosc_30.csv" lt rgb "#7e2f8e" pt 11 ps 0.3 notitle, \
    1/0 with point lt rgb "#77ac30" pt 13 lw 3 title "Intensywność napływu wiadomości podstawowych = 9, średnia długość wiadomości podstawowej = 30 bajtów", "natezenie_9_dlugosc_30.csv" lt rgb "#77ac30" pt 13 ps 0.3 notitle, \
    1/0 with point lt rgb "#a2142f" pt 15 lw 3 title "Intensywność napływu wiadomości podstawowych = 1, średnia długość wiadomości podstawowej = 400 bajtów", "natezenie_1_dlugosc_400.csv" lt rgb "#a2142f" pt 15 ps 0.3 notitle

# plot 'natezenie_1_dlugosc_100.csv' lt rgb "#0072bd" pt 5   ps 0.3 title "Intensywność napływu wiadomości podstawowych = 1, średnia długość wiadomości podstawowej = 100 bajtów", \
#      'natezenie_5_dlugosc_100.csv' lt rgb "#d95319" pt 9   ps 0.3 title "Intensywność napływu wiadomości podstawowych = 5, średnia długość wiadomości podstawowej = 100 bajtów", \
#      'natezenie_9_dlugosc_100.csv' lt rgb "#edb120" pt 7   ps 0.3 title "Intensywność napływu wiadomości podstawowych = 9, średnia długość wiadomości podstawowej = 100 bajtów", \
#      'natezenie_5_dlugosc_30.csv'  lt rgb "#7e2f8e" pt 11  ps 0.3 title "Intensywność napływu wiadomości podstawowych = 5, średnia długość wiadomości podstawowej = 30 bajtów", \
#      'natezenie_9_dlugosc_30.csv'  lt rgb "#77ac30" pt 13  ps 0.3 title "Intensywność napływu wiadomości podstawowych = 9, średnia długość wiadomości podstawowej = 30 bajtów", \
#      'natezenie_1_dlugosc_400.csv' lt rgb "#a2142f" pt 15  ps 0.3 title "Intensywność napływu wiadomości podstawowych = 1, średnia długość wiadomości podstawowej = 400 bajtów" 
