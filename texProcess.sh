#!/bin/bash
FILE="${1##*/}"
FILE="res/${FILE%%.*}.png"
convert $1 -resize 24x24 $FILE
convert $FILE -brightness-contrast -12,-10 "${FILE%%.*}_bg.png"
convert $FILE res/mask_br.png -alpha off -compose copy_opacity -composite "${FILE%%.*}_br.png"
convert $FILE res/mask_bl.png -alpha off -compose copy_opacity -composite "${FILE%%.*}_bl.png"
convert $FILE res/mask_tr.png -alpha off -compose copy_opacity -composite "${FILE%%.*}_tr.png"
convert $FILE res/mask_tl.png -alpha off -compose copy_opacity -composite "${FILE%%.*}_tl.png"
