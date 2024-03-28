# example.gnuplut : configuration for plotting (change as needed)

reset                                   # reset
set size ratio 0.2                      # set relative size of plots
set grid xtics ytics                    # grid: enable both x and y lines
set grid lt 1 lc rgb '#cccccc' lw 1     # grid: thin gray lines
set multiplot layout 2,1 scale 1.0,1.0  # set two plots for this figure

# time domain
set ylabel 'Sample value'               # set y-axis label
set xlabel 'Sample #'                   # set x-axis label
set yrange [-1.5:1.5]                       # set y plot range
set xrange [0:1000]                      # set x plot range
plot '../data/rds_check.dat' using 1:2 with lines lt 1 lw 2 lc rgb '#000088' notitle, \
'../data/rds_impulse.dat' using 1:2 with lines lt 1 lw 2 lc rgb '#aa0000' notitle


set ylabel 'Sample value'               # set y-axis label
set xlabel 'Sample #'                   # set x-axis label
set yrange [-2:2]                       # set y plot range
set xrange [0:1000]                      # set x plot range
plot '../data/rds_clean.dat' using 1:2 with lines lt 1 lw 2 lc rgb '#000088' notitle
