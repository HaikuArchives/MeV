#!/bin/sh
set -e

mkdir -p app/add-ons
cd src
make $@
cd ../add-ons/SMF
make $@
cd ../../
