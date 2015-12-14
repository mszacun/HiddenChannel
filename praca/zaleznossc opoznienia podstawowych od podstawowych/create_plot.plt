set terminal svg enhanced font 'Verdana,8'
set output './podstawowych_od_podstawowych.svg'

set title "Zależność średniego opóźnienia wiadomości podstawowych\n od intensywności napływu wiadomości podstawowych"
set xlabel 'Intensywność napływu wiadomości podstawowych'
set ylabel 'Średnie opóźnienie wiadomości podstawowych'
plot '4bity.csv'   lt rgb "#0072bd" pt 5 ps 1 title 'Długość segmentu wiadomości ukrytej = 4 bity', \
     '8bitow.csv'  lt rgb "#d95319" ps 1 pt 9 title 'Długość segmentu wiadomości ukrytej = 8 bitów', \
     '10bitow.csv' lt rgb "#edb120" pt 7 ps 1 title 'Długość segmentu wiadomości ukrytej = 10 bitów' 

