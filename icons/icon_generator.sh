#!/usr/bin/env bash

for i in {16,32,48,64,128,256};do
    inkscape  scalable/io.github.glaumar.QRookie.svg -w $i -h $i -o "${i}x${i}/io.github.glaumar.QRookie.png"
done
