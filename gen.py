#!/usr/bin/python3

import glob, subprocess as subp, sys

# this is bad code im just lazy
# this script generates the build.ninja file,
# you need to re-run it when a new file is created/deleted

arch = 'x86_64'
if len(sys.argv) > 1: arch = sys.argv[1]

opt = {
    "asm": True
}

print("generating for " + arch)

# kernel sources
src_kernel = {
    'cc': glob.glob(f'kernel/{arch}/*.c')
        + glob.glob(f'kernel/dev/*.c')
        + glob.glob(f'kernel/opt/*.c')
        + glob.glob(f'kernel/*.c'),
    'as': glob.glob(f'kernel/{arch}/*.s'),
}

# userspace sources
src_usr = {
    'cc': glob.glob(f'usr/lib/{arch}/*.c')
        + glob.glob(f'usr/lib/*.c')
        + glob.glob(f'usr/*.c'),
    'as': glob.glob(f'usr/lib/{arch}/*.s')
}

# library files that are used when linking a progam
usr_link = [
    *glob.glob('usr/lib/*.c'),
    *glob.glob('usr/lib/*.s'),
    *glob.glob(f'usr/lib/{arch}/*.c'),
    *glob.glob(f'usr/lib/{arch}/*.s'),
]

# userspace program files (each file - one program)
usr_prog = glob.glob('usr/*.c')

# deps. for `boot/`
boot_files = []
okernel = []

with open('kernel/build.ninja', 'w') as fout:
    fout.write('# generated automatically, edit gen.py instead\n')
    fout.write('include kernel/rules.ninja\n')

    for k, fs in src_kernel.items():
        for f in fs:
            if f[0] == '~': continue # skip ~backup.x files
            fout.write('build build/' + f + '.o: ' + k + ' ' + f + '\n')
            okernel.append('build/' + f + '.o')

    fout.write('\n')
    fout.write(f'build boot/kernel.elf: ld ' + ' '.join(okernel) + '\n')
    boot_files.append('boot/kernel.elf')

with open('usr/build.ninja', 'w') as fout:
    fout.write('# generated automatically, edit gen.py instead\n')
    fout.write('include usr/rules.ninja\n')

    for k, fs in src_usr.items():
        for f in fs:
            if f[0] == '~': continue # skip ~backup.x files
            fout.write(f'build build/{f}.o: {k} {f}\n')

    oprog = [(f'boot/{f}.bin', f'build/{f}.o') for f in usr_prog]
    olink = [f'build/{f}.o' for f in usr_link]
    output = [x[0] for x in oprog]
    
    for bin, obj in oprog:
        fout.write(f'build {bin}: ld {obj} {" ".join(olink)}\n')
        # fout.write(f'  flags = {" ".join(f"--just-symbols {x}" for x in okernel)}\n')
    
    boot_files += output

with open('build.ninja', 'w') as fout:
    fout.write('# generated automatically, edit gen.py instead\n')
    fout.write(f'platformdef = -DNANOS_PLATFORM_{arch.upper()} {"-DNANOS_X86_64_OPT_ASM" if opt["asm"] else ""}\n')
    fout.write('include rules.ninja\n')
    fout.write('subninja kernel/build.ninja\n')
    fout.write('subninja usr/build.ninja\n')
    deps = ' '.join(glob.glob('boot/*'))
    fout.write(f'build nanos.iso: sh geniso.sh {" ".join(boot_files)} | {deps}\n')
    fout.write(f'default nanos.iso\n')

with open('compile_commands.json', 'w') as fout:
    fout.write(subp.getoutput('ninja -t compdb'))
    fout.write('\n')
