#!/usr/bin/python3

import difflib
import errno
from io import BufferedReader
import os
from pathlib import Path
import subprocess
import sys

def obj_flatten(inputfile: Path, outputfile: Path, **kwargs):
    return subprocess.run([
        'llvm-objcopy', '-O', 'binary',
        inputfile, outputfile
    ], **kwargs)

def binary_file_diff(fileone: BufferedReader, filetwo: BufferedReader):
    d = difflib.Differ()
    diff = d.compare(
        list(map(hex, fileone.read())),
        list(map(hex, filetwo.read()))
    )
    return [line for line in diff if not line.startswith('  ')]

def delete_file(file: Path):
    try:
        os.remove(file)
    except OSError as e:
        if e.errno == errno.ENOENT:
            return
        raise

def execute(wrasmexe: Path, asmfile: Path, id: str):
    wrasmobj = Path(f'{id}.wrasm.o')
    llvmobj = Path(f'{id}.llvm.o')
    flatwrasm = Path(f'{id}.flatwrasm')
    flatllvm = Path(f'{id}.flatllvm')

    subprocess.run([
        wrasmexe, asmfile,
        '-o', wrasmobj
    ], check=True)
    subprocess.run([
        'llvm-mc',
        '--triple=riscv64-unknown-elf',
        '-filetype=obj',
        asmfile,
        '-o', llvmobj
    ], check=True)

    obj_flatten(wrasmobj, flatwrasm, check=True)
    obj_flatten(llvmobj, flatllvm, check=True)

    with open(flatwrasm, 'rb') as w:
        with open(flatllvm, 'rb') as l:
            diff = binary_file_diff(w, l)

    if not diff:
        return

    print(diff)
    raise Exception("Difference in Resulting Files")

def cleanup(id: str):
    delete_file(Path(f'{id}.wrasm.o'))
    delete_file(Path(f'{id}.llvm.o'))
    delete_file(Path(f'{id}.flatwrasm'))
    delete_file(Path(f'{id}.flatllvm'))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Incorrect number of arguments")
        exit(1)

    wrasmexe = Path(sys.argv[1])
    asmfile = Path(sys.argv[2])
    id = f'{os.getpid()}_{asmfile.name}'

    try:
        execute(wrasmexe, asmfile, id)
    finally:
        cleanup(id)
