#!/usr/bin/python

size = 251
l = []
for i in xrange(size):
    l.append(i)
for i in xrange(size):
    l.append(size - i)
print l, len(l)
