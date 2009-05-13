#!/bin/sh

if [ -e opencore ]; then
	echo The directory opencore already exists, doing nothing
	exit 1
fi

git clone git://android.git.kernel.org/platform/external/opencore.git

cd opencore
# original version, well tested
#git checkout 5c99f4418406c6400ddbcdce46736c30d21415dc
# latest, seems to work fine too
git checkout 6b31782f62c033186ad890719195f3c04e0a5f25
git checkout -b local_patches
for i in ../patches/*; do
	patch -p1 < $i
	git commit -a -m `basename $i`
done

