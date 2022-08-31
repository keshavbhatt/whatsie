#!/bin/bash
FILE=$1
WIDTH=$(identify -format '%w' $FILE)
HEIGHT=$(identify -format '%h' $FILE)
NAME=${1%%.*}
EXT=${1#*.}
W=$(echo $WIDTH/2.2 | bc);
H=$W;
X=$(echo $WIDTH-$W | bc)
Y=$(echo $HEIGHT-$H | bc)
RY=$(echo $W/8 | bc);
TX=$(echo $W/2 | bc);
TY=$(echo $H-$RY | bc);
FS=$(echo $W+$RY | bc);

echo "File: $FILE"
echo "Dimensions: $WIDTH x $HEIGHT"
echo "Name: $NAME"
echo "Ext: $EXT"
echo "X: $X"
echo "Y: $Y"
echo "RY: $RY"
echo "W: $W"
echo "H: $H"
echo "TX: $TX"
echo "TY: $TY"
echo "FS: $FS"

function makeicon {
  echo "Creating.. $1 into $2."
  convert -background none -page +0+0 $FILE -page +0+0 <(cat << EOF
<?xml version="1.0" encoding="UTF-8"?>
<svg  width="$WIDTH" height="$HEIGHT" version="1.1" xmlns="http://www.w3.org/2000/svg">
  <g transform="translate($X,$Y)">
    <rect x="0" y="0" width="$W" height="$H" ry="$RY" fill="#ff0000" stroke="#ff0000"/>
    <text x="$TX" y="$TY" dominant-baseline="top" text-anchor="middle" fill="#ffffff" stroke="#cccccc" font-family="Ubuntu" font-size="$FS">$1</text>
  </g>
</svg>
EOF
) -layers merge $2
}

for i in {1..9}
do
  makeicon $i "$NAME-notify-$i.$EXT"
done
makeicon + "$NAME-notify-10.$EXT"
