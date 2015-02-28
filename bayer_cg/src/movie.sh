#!/bin/sh

mencoder -ovc lavc -lavcopts vcodec=mpeg4 -mf type=png:fps=29.97 -nosound -o output.avi mf://out/\*.png
