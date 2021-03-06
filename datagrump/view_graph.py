#!/usr/bin/env python

import time
import subprocess
import sys
from Queue import Queue
from threading import Thread

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

filename = sys.argv[1]
print filename



start_time = time.time()
times, values = [], []
TIME_WINDOW = 5

AXIS_FACTOR = 0.25

data_queue = Queue()
def tail():
    f = subprocess.Popen(['tail', '-n', '1', '-F',filename],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    while True:
        value = float(f.stdout.readline())
        current_time = time.time() - start_time
        data_queue.put((current_time, value))
Thread(target=tail).start()

def animate(frameno):
    while not data_queue.empty():
        (current_time, value) = data_queue.get()
        times.append(current_time)
        values.append(value)

    current_time = time.time() - start_time
    while len(times) > 0 and times[0] < current_time - TIME_WINDOW:
        times.pop(0)
        values.pop(0)

    line.set_data(times, values)
    max_val, min_val = max(values), min(values)
    ax.set_ylim(min_val - abs(min_val)*AXIS_FACTOR, max_val + abs(max_val)*AXIS_FACTOR)
    ax.set_xlim(current_time - TIME_WINDOW, current_time)

fig, ax = plt.subplots()
plt.title(filename)
[line] = ax.plot([0], [0])
ani = animation.FuncAnimation(fig, animate, blit=False, interval=10, repeat=True)
plt.show()
