#!/bin/bash
ffmpeg -y -i $1 -t 10 -pix_fmt yuv420p -profile:v baseline -vf scale=1280:720:out_color_matrix=bt709 -b:v 4M $1-720p25-4M-bt709.mp4
ffmpeg -y -i $1 -t 10 -pix_fmt yuv420p -profile:v baseline -vf scale=1920:1080:out_color_matrix=bt709 -b:v 8M $1-1080p25-8M-bt709.mp4
ffmpeg -y -i $1 -t 10 -pix_fmt yuv420p -profile:v baseline -vf scale=3840:2160:out_color_matrix=bt709 -b:v 16M $1-2160p25-16M-bt709.mp4
