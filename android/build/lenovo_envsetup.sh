#!/bin/bash
project=$1

export LENOVO_PRODUCT_ROOT="vendor/lenovo"

echo "LENOVO_PRODUCT_ROOT=${LENOVO_PRODUCT_ROOT}"

for f in `/bin/ls ${LENOVO_PRODUCT_ROOT}/lenovosetup.sh ${LENOVO_PRODUCT_ROOT}/${project}/lenovosetup.sh 2> /dev/null`
do
    echo "including $f"
    . $f
done
unset f

