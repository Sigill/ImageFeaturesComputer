#!/bin/sh

THISDIR=$(dirname $0)

export LD_LIBRARY_PATH=${THISDIR}:${LD_LIBRARY_PATH}
export LC_NUMERIC=C

$THISDIR/features_computer $@
