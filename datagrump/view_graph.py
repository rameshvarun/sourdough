import time
import subprocess
import select
import sys

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

filename = sys.argv[1]
print filename

f = subprocess.Popen(['tail', '-n', '1', '-F',filename],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
p = select.poll()
p.register(f.stdout)

start_time = time.time()
times, values = [], []
TIME_WINDOW = 10
DELETE_WINDOW = 20

AXIS_FACTOR = 0.25

def animate(frameno):
    while p.poll(0):
        value = float(f.stdout.readline())
        current_time = time.time() - start_time

        times.append(current_time)
        values.append(value)

        line.set_data(times, values)
        max_val, min_val = max(values), min(values)
        ax.set_ylim(min_val - abs(min_val)*AXIS_FACTOR, max_val + abs(max_val)*AXIS_FACTOR)
        ax.set_xlim(current_time - TIME_WINDOW, current_time)

        while len(times) > 0 and times[0] < current_time - DELETE_WINDOW:
            times.pop(0)
            values.pop(0)

fig, ax = plt.subplots()
plt.title(filename)
[line] = ax.plot([0], [0])
ani = animation.FuncAnimation(fig, animate, blit=False, interval=10, repeat=True)
plt.show()
