#!/bin/bash
set -e
set -u
set -o pipefail

LIBDIR=libs

function build_otf {
	echo "downloading ..."
	wget -nc http://tu-dresden.de/die_tu_dresden/zentrale_einrichtungen/zih/forschung/projekte/otf/dateien/OTF-1.12.5salmon.tar.gz

	echo "extracting ..."
	tar xzf OTF-1.12.5salmon.tar.gz

	echo "configure ..."
	local DEST=$(pwd)/$LIBDIR
	(cd OTF-1.12.5salmon && ./configure --prefix=$DEST)

	echo "building ..."
	(cd OTF-1.12.5salmon && make -j 4)

	echo "installing ..."
	(cd OTF-1.12.5salmon && make install)
}

if [ ! -d $LIBDIR ]; then
	mkdir $LIBDIR
fi

if [ ! -d $LIBDIR/otf ]; then
	echo "=================="
	echo "Building OTF"
	echo "=================="
	build_otf
else
	echo "Skipping OTF build, exists already."
fi
