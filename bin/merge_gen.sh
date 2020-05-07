#!/bin/bash

cat $* | awk ' { c[$1] += $2 } END { for (v in c) print v, c[v] }' | sort -n
