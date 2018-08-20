set datafile separator ","
plot "mag.csv" using 1:2 title "XY" pointsize 2 pointtype 7, "mag.csv" using 3:1 title "ZX" pointsize 2 pointtype 7, "mag.csv" using 3:2 title "ZY" pointsize 2 pointtype 7
