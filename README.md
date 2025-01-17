# Nanos
This is my personal attempt at making a *functional* kernel. If you didn't know
already, I made an operating system in C a few months ago. But uhhh, the kernel
was built upon very weak glue. Like, something you would buy at the supermarket!
So yeah, this is my *real* attempt at making a good kernel.

## Requirements
* [clang](https://clang.llvm.org/)
* [limine](https://limine-bootloader.org/)
* [nasm](https://nasm.us/)
* [ninja](https://ninja-build.org/)
* [python3](https://www.python.org/)

## Install
```sh
git clone https://github.com/ElisStaaf/nanos
cd nanos

./gen.py
ninja
qemu-system-x86_64 out.iso -serial stdio
```
