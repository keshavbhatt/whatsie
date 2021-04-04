#!/bin/bash

# Edit the settings below:

# Output sizes - 
# Please note the format: each size is wrapped with quotes, width and height are separated with space.
output=("16 16" "32 32" "64 64" "128 128" "256 256" "512 512" )

# If you frequently use the same source file (f.e. "~/Desktop/src.jpg"), 
# set it in "default_src"
default_src="";

# If you frequently use the same destination
# (f.e. "~/Desktop/Some_folder/%.jpg"), set it in "default_dst"
# Destination must include "%", it will be replaced by output size, f.e. "800x600"
default_dst="%.png";

# Add signature? 
default_sign='n'

# If you frequently use the same signature file (f.e. "~/Desktop/sig.png"), 
# set it in "default_sig"
default_sig="";

# Gravity is for cropping left/right edges for different proportions (center, east, west)
default_gravity="center"

# Output JPG quality: maximum is 100 (recommended)
quality=100

# ======
# Do not edit below.
# ======
# Welcome to Smashing resizer!
# Written by Vlad Gerasimov from http://www.vladstudio.com
# 
# This script takes one "source" image and saves it in different sizes.
# 
# Requires:
# * imagemagick - http://www.imagemagick.org/
# * python - I'm sure your PC already has it!



# Unfortunately, bash is awful at math operations.
# We'll create a simple function that handles math for us.
# Example: $(math 2 * 2)

function math(){
	echo $(python -c "from __future__ import division; print $@")
}



# To make our script short and nice, here is the "save()" function.
# We'll use it to save each size.

function save(){

	# read target width and height from function parameters
	local dst_w=${1}
	local dst_h=${2}

	# calculate ratio 
	local ratio=$(math $dst_w/$dst_h);

	# calculate "intermediate" width and height
	local inter_w=$(math "int(round($src_h*$ratio))")
	local inter_h=${src_h}

	# which size we're saving now
	#local size="${dst_w}x${dst_h}"
	local size="${dst_w}"
	echo "Saving ${size}..."

	#crop intermediate image (with target ratio)
	convert ${src} -gravity ${gravity} -crop ${inter_w}x${inter_h}+0+0 +repage temp.psd

	# apply signature
	if [ "${sign}" == "y" ]; then
	convert temp.psd ${sig} -gravity southeast -geometry ${sig_w}x${sig_h}+24+48 -composite temp.psd
	fi

	## For best quality, I resize image 80%, sharpen 3 times, then repeat.

	# setup resize filter and unsharp parameters (calculated through trial and error)
	local arguments="-interpolate bicubic -filter Lagrange"
	local unsharp="-unsharp 0.4x0.4+0.4+0.008"

	# scale 80%, sharpen, repeat until less than 150% of target size
	local current_w=${dst_w}

	# pardon moi for such ugly while condition!
	while [ $(math "${current_w}/${dst_w} > 1.5") = "True" ]; do
		current_w=$(math ${current_w}\*0\.80)
		current_w=$(math "int(round(${current_w}))")
		arguments="${arguments} -resize 80% +repage ${unsharp} ${unsharp} ${unsharp} "
	done

	# final resize
	arguments="${arguments} -resize ${dst_w}x${dst_h}! +repage ${unsharp} ${unsharp} ${unsharp} -density 72x72 +repage"

	# final convert! resize, sharpen, save
	convert temp.psd ${arguments} -quality ${quality} ${dst/\%/${size}}

}

# Ask for source image, or use default value
echo "Enter path to source image, or hit Enter to keep default value (${default_src}): "
read src
src=${src:-${default_src}}

# ask for destination path, or use default value
echo "Enter destination path, or hit Enter to keep default value (${default_dst})."
echo "must include % symbol, it will be replaced by output size, f.e. '800x600'"
read dst
dst=${dst:-${default_dst}}

# Ask for signature image, or use default value
echo "Apply signature? (hit Enter for 'yes' or type 'n' for 'no') "
read sign
sign=${sign:-${default_sign}}

# Ask for signature image, or use default value
echo "Enter path to signature image, or hit Enter to keep default value (${default_sig}): "
read sig
sig=${sig:-${default_sig}}

# ask for gravity, or use default value
echo "Enter gravity for cropping left/right edges (center, east, west), or hit Enter to keep default value (${default_gravity}): "
read gravity
gravity=${gravity:-${default_gravity}}


# detect source image width and height
src_w=$(identify -format "%w" "${src}")
src_h=$(identify -format "%h" "${src}")

# detect signature width and height
if [ "${sign}" == "y" ]; then
	sig_w=$(identify -format "%w" "${sig}")
	sig_h=$(identify -format "%h" "${sig}")
fi

# loop throught output sizes and save each size
for i in "${output[@]}"
do
	save ${i}
done

# Delete temporary file
rm temp.psd

# Done!
echo "Done!"
