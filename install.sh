#!/bin/bash

set -e

if ! [[ "$UID" -eq "0" ]]; then
    echo "script should be runned as root"
    exit 1
fi

INSTALL_DIR="/opt/hob"

if [[ "$1" = "uninstall" ]]; then
    rm -Rf $INSTALL_DIR
    echo "Uninstalled successfully!"
    echo "Do not forget to remove /opt/hob/bin from your PATH variable!"
else
    mkdir -p $INSTALL_DIR/bin
    cp ./hoblang $INSTALL_DIR/bin
    cp -r ./lib $INSTALL_DIR
    echo "Installed successfully!"
    echo "Add /opt/hob/bin in your PATH variable!"
fi
