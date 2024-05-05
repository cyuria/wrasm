#!/usr/bin/python3
# *_* coding: utf-8 *_*

"""
System Testing Python Script for Wrasm

This script requires la built copy of Wrasm, the llvm linker (lld) and the
userspace qemu riscv64 emulation binaries.

Each of these can be manually specified using the following environment
variables or found automatically.

wrasm: the "WRASM" environment variable
lld: the "LLD" environment variable
qemu: the "QEMU_RISCV64" environment variable

The wrasm binaries are located by recursively searching for a file named
"wrasm", starting with the project root directory
"""

from difflib import Differ
from errno import ENOENT
from json import load
from os import environ, getpid, remove
from pathlib import Path
from platform import system
from shutil import which
from subprocess import run
from sys import argv

script = Path(__file__).resolve()

linkers = {
    'Windows': 'lld-link',
    'Darwin': 'ld64.lld',
    'Linux': 'ld.lld',
}

def find_qemu() -> Path:
    if environ.get('QEMU_RISCV64'):
        qemu = Path(environ['wrasm']).resolve()
        print(f"qemu binary found (environment): {qemu}")
        return qemu

    binaries = [ 'qemu-riscv64', 'qemu-riscv64-static' ]
    try:
        qemu = next(
            Path(exe) for exe in map(which, binaries)
            if exe is not None
        )
        print(f"qemu binary found (automatic): {qemu}")
        return qemu
    except StopIteration:
        raise Exception(
            "Unable to find qemu riscv64 userspace emulation binaries on path"
        )

def find_wrasm() -> Path:
    if environ.get('WRASM'):
        wrasm = Path(environ['wrasm']).resolve()
        print(f"wrasm binary found (environment): {wrasm}")
        return wrasm

    try:
        project_root = next(
            d for d in script.parents
            if (d / '.git').is_dir()
        )
    except StopIteration:
        raise Exception("Unable to find root project directory,"
            "try specifying the location of the wrasm executable"
            "manually via the \"WRASM\" environment variable")

    for f in Path(project_root).rglob('wrasm'):
        if not f.is_file():
            continue
        if not f.stat().st_mode & 0o100:
            continue
        wrasm = f.resolve()
        print(f"wrasm binary found (automatic): {wrasm}")
        return wrasm

    raise Exception(
        "Unable to find compiled wrasm binary"
    )

def find_lld() -> Path:
    if environ.get('LLD'):
        lld = Path(environ['LLD']).resolve()
        print(f"lld binary found (environment): {lld}")
        return lld

    lld = which(linkers[system()])
    if lld is None:
        raise Exception("Unable to find lld (the llvm linker) binaries on path")
    lld = Path(lld)
    print(f"lld binary found (automatic): {lld}")
    return lld

def delete_file(file: Path):
    try:
        remove(file)
    except OSError as e:
        if e.errno == ENOENT:
            return
        raise

def compile(wrasm: Path, lld: Path, qemu: Path, asm: Path, id: str) -> str:
    obj = Path(f'{id}.o').resolve()
    exe = Path(f'{id}').resolve()

    run([ wrasm, asm, '-o', obj ], check=True)
    run([ lld, obj, '-o', exe ], check=True)
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

def test(wrasm: Path, lld: Path, qemu: Path, id: str, t: dict[str, str]):

    efpath = script.parent / t['expected']

    with open(efpath, 'r') as f:
        expected = f.read()

    try:
        output = compile(wrasm, lld, qemu, script.parent / t['asm'], id)
        compare(expected, output)
        print(f"Test completed | {t['name']} ({t['asm']})")
    finally:
        cleanup(id)

def testall():
    wrasm = find_wrasm()
    lld = find_lld()
    qemu = find_qemu()

    index = (script.parent / 'tests.json').resolve()
    with open(index) as f:
        tests = load(f)

    pid = getpid()
    print("Tests Loaded")
    print()

    print("Testing...")
    for tid, t in enumerate(tests):
        id = f"{pid}_{tid}_{t['name']}"
        test(wrasm, lld, qemu, id, t)

def main(args=[]):
    if any(cmd in args for cmd in ['help', '--help', '-h']):
        print(__doc__)
        return
    testall()

if __name__ == "__main__":
    main(argv[1:])

