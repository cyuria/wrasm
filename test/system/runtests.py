#!/usr/bin/python3

from difflib import Differ
import errno
from json import load
import os
from pathlib import Path
from platform import system
from shutil import which
from subprocess import run, DEVNULL
from sys import argv

dir = Path(__file__).resolve().parent

linkers = {
    'Windows': 'lld-link',
    'Darwin': 'ld64.lld',
    'Linux': 'ld.lld',
}

def find_qemu() -> Path:
    binaries = [ 'qemu-riscv64', 'qemu-riscv64-static' ]
    try:
        return next(
            Path(exe) for exe in map(which, binaries)
            if exe is not None
        )
    except StopIteration:
        raise Exception(
            "Unable to find qemu riscv64 userspace emulation binaries on path"
        )

def find_wrasm():
    if len(argv) == 2:
        return Path(argv[1]).resolve()

    for f in Path('../../').rglob('wrasm'):
        if not f.is_file():
            continue
        if not f.stat().st_mode & 0o100:
            continue
        return f
    raise Exception(
        "Unable to find compiled wrasm binary"
    )

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
    run([ linker, obj, '-o', exe ], check=True)
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

def test(id: str, t: dict[str, str]):

    efpath = dir / t['expected']

    with open(efpath, 'r') as f:
        expected = f.read()

    try:
        output = compile(dir / t['asm'], id)
        compare(expected, output)
    finally:
        cleanup(id)

def testall():
    index = (dir / 'tests.json').resolve()
    with open(index) as f:
        tests = load(f)

    pid = os.getpid()

    for tid, t in enumerate(tests):
        id = f"{pid}_{tid}_{t['name']}"
        test(id, t)

if __name__ == "__main__":
    qemu = find_qemu()
    wrasm = find_wrasm()
    linker = which(linkers[system()])
    testall()

