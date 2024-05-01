#!/usr/bin/python3

from difflib import Differ
import errno
import os
from pathlib import Path
from shutil import which
from subprocess import run
from sys import argv

def delete_file(file: Path):
    try:
        os.remove(file)
    except OSError as e:
        if e.errno == errno.ENOENT:
            return
        raise

def compile(asm: Path, id: str) -> str:
    obj = Path(f'{id}.o').resolve()
    exe = Path(f'{id}').resolve()

    run([ wrasm, asm, '-o', obj ], check=True)
    run([ 'ld.lld', obj, '-o', exe ], check=True)
    result = run([ qemu, exe ], capture_output=True, text=True, check=True)

    return result.stdout

def compare(expected: str, output: str):
    d = Differ()

    diff = d.compare(expected.split('\n'), output.split('\n'))

    if not list(filter(lambda l: not l.startswith('  '), diff)):
        return

    print('\n'.join(diff))
    raise Exception("Difference in Resulting Files")

def cleanup(id: str):
    delete_file(Path(f'{id}.o'))
    delete_file(Path(f'{id}'))

if __name__ == "__main__":
    if len(argv) != 4:
        print("Incorrect number of arguments")
        exit(99)

    qemu = Path(argv[1])
    wrasm = Path(argv[2])
    input = Path(argv[3])
    id = f'{os.getpid()}_{input.name}'
    qemu = which('qemu-riscv64')

    if qemu is None:
        print("`qemu-riscv64` user space executable not found")
        exit(77)

    with open(input.with_suffix('.txt'), 'r') as f:
        expected = f.read()

    try:
        output = compile(input.with_suffix('.S'), id)
        compare(expected, output)
    finally:
        cleanup(id)
