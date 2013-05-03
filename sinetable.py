#!/usr/bin/python
from __future__ import division
import math

pislice = math.pi/1000
sintable = []

for x in xrange(1000):
    sintable.append(int(math.sin(x*pislice) * 255))

print sintable 
