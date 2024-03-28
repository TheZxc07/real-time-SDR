import numpy as np
from scipy import signal
x = np.zeros(100)
for n in range(len(x)):
    x[n]= n

y = np.zeros(100)
y[50] = 1

z = signal.lfilter(y, 1.0, x)
print(z)