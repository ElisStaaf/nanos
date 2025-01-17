#!/usr/bin/bash

mkdir -p boot
cp $LIMINE_PATH/limine-cd.bin boot || exit 1
cp $LIMINE_PATH/limine-cd-efi.bin boot || exit 1
cp $LIMINE_PATH/limine.sys boot || exit 1
tar -c boot/usersp > boot/init.tar

xorriso -as mkisofs -b limine-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    --efi-boot limine-cd-efi.bin \
    -efi-boot-part --efi-boot-image --protective-msdos-label \
    boot -o ${@:$#} > /dev/null 2>&1 || exit 1

$LIMINE_PATH/limine-deploy ${@:$#} || exit 1
