# example.gnuplut : configuration for plotting (change as needed)

reset                                   # reset
set size ratio 0.2                      # set relative size of plots
set grid xtics ytics                    # grid: enable both x and y lines
set grid lt 1 lc rgb '#cccccc' lw 1     # grid: thin gray lines
set multiplot layout 2,1 scale 1.0,1.0  # set two plots for this figure

# time domain
set ylabel 'Sample value'               # set y-axis label
set xlabel 'Sample #'                   # set x-axis label
set yrange [-1:1]                       # set y plot range
set xrange [5000:5300]                      # set x plot range
plot '../data/NCO.dat' using 1:2 with lines lt 1 lw 2 lc rgb '#000088' notitle

# freq domain (Fourier)
# time domain
set ylabel 'Sample value'               # set y-axis label
set xlabel 'Sample #'                   # set x-axis label
set yrange [-0.01:0.01]                       # set y plot range
set xrange [5000:5300]                      # set x plot range
plot '../data/PLL.dat' using 1:2 with lines lt 1 lw 2 lc rgb '#000088' notitle

unset multiplot
