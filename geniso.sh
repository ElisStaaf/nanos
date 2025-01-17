mkdir -p boot

cp $LIMINE_PATH/limine-uefi-cd.bin boot || exit 1
cp $LIMINE_PATH/limine-bios.sys boot || exit 1

tar -c boot/usr > boot/init.tar

xorriso -as mkisofs \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    --efi-boot limine-uefi-cd.bin \
    -efi-boot-part --efi-boot-image --protective-msdos-label \
    boot -o ${@:$#} > /dev/null 2>&1 || exit 1

$LIMINE_PATH/limine-deploy ${@:$#} > /dev/null 2>&1 || exit 1
