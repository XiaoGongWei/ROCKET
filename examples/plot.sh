#!/bin/bash
#
ls pop.inpl.new | while read line
do

   gnuplot << EOF
   set terminal png
   set output '${line}.png'
   set xlabel 'day'
   set ylabel 'diff(m)'
   set size 1.0, 1.0
   set grid
   set yrange[-0.5:0.5]
   plot "$line" using 1:2  with lines title "dn", \
        "$line" using 1:3  with lines title "de", \
        "$line" using 1:4  with lines title "du"
EOF

done
