set terminal svg enhanced font 'Verdana,9'
set output './ukrytych_od_podstawowych.svg'

set title "Zależność średniego opóźnienia wiadomości ukrytych\n od intensywności napływu wiadomości podstawowych"
set xlabel 'Intensywność napływu wiadomości podstawowych'
set ylabel 'Średnie opóźnienie wiadomości ukrytych'

plot [:15] 'dlugosc_podstawowych_50.csv'  lt rgb "#0072bd" pt 5   ps 0.7 title "Średnia długość wiadomości podstawowych = 50", \
     [:15] 'dlugosc_podstawowych_100.csv' lt rgb "#d95319" pt 9   ps 0.7 title "Średnia długość wiadomości podstawowych = 100", \
     [:15] 'dlugosc_podstawowych_150.csv' lt rgb "#edb120" pt 7   ps 0.7 title "Średnia długość wiadomości podstawowych = 150", \
