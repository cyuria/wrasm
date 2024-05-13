#!/usr/bin/env python3

"""System Testing Python Script for Wrasm

This script requires la built copy of Wrasm, the llvm linker (lld) and the
userspace qemu riscv64 emulation binaries.

Each of these can be manually specified using the following environment
variables or found automatically.

wrasm: the "WRASM" environment variable
lld: the "LLD" environment variable
qemu: the "QEMU_RISCV64" environment variable

The wrasm binaries are located by recursively searching for a file named
"wrasm", starting with the project root directory"""

from difflib import Differ
from errno import ENOENT
from json import load
from multiprocessing import Pool
from os import environ, getpid, remove
from pathlib import Path
from platform import system
from shutil import which
from subprocess import run
from sys import argv, exit

class ansi:
    OK = "\033[92m"
    FAIL = "\033[91m"
    RESET = "\033[0m"

script = Path(__file__).resolve()

linkers = {
    'Windows': 'lld-link',
    'Darwin': 'ld64.lld',
    'Linux': 'ld.lld',
}

def find_qemu() -> Path:
    if 'QEMU_RISCV64' in environ:
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
    if 'WRASM' in environ:
        wrasm = Path(environ['WRASM']).resolve()
        print(f"wrasm binary found (environment): {wrasm}")
        return wrasm

    try:
        project_root = next(
            d for d in script.parents
            if (d / '.git').is_dir()
        )
    except StopIteration:
        raise Exception("Not a git repository, "
            "try specifying the location of the wrasm executable "
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
    if 'LLD' in environ:
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

def execute(asm: Path, id: str) -> str:
    obj = Path(f'{id}.o').resolve()
    exe = Path(f'{id}').resolve()

    run([ wrasm, asm, '-o', obj ], check=True)
    run([ lld, obj, '-o', exe ], check=True)
    result = run([ qemu, exe ], capture_output=True, text=True, check=True)

    return result.stdout

def compare(expected: str, output: str):
    diff = [
        l for l in
        Differ().compare(
            expected.split('\n'),
            output.split('\n')
        )
        if not l.startswith('  ')
    ]

    if diff:
        print('\n'.join(diff))
        raise Exception("Unexpected Output")

def cleanup(id: str):
    delete_file(Path(f'{id}.o'))
    delete_file(Path(f'{id}'))

def test(id: str, t: dict[str, str]):

    with open(script.parent / t['expected'], 'r') as f:
        expected = f.read()

    try:
        output = execute(script.parent / t['asm'], id)
        compare(expected, output)
    finally:
        cleanup(id)

def runtest(args) -> tuple[bool, str]:
    i, t = args
    id = f"{pid}_{i}_{t['name']}"
    try:
        test(id, t)
    except Exception as e:
        result = False
        status = f"{ansi.FAIL}FAIL{ansi.RESET} - {e}"
    else:
        result = True
        status = f"{ansi.OK}OK{ansi.RESET}"
    return result, f" | {t['name']} | {status}"

def testall() -> int:

    with open((script.parent / 'tests.json').resolve()) as f:
        tests = load(f)

    with Pool() as pool:
        results, statuses = zip(*pool.map(runtest, enumerate(tests)))

    print(*statuses, sep='\n')
    failed = results.count(False)
    passed = len(tests) - failed

    print(
        f"{ansi.FAIL if failed else ansi.OK}"
        f"{passed}/{len(tests)} tests passed"
        f"{ansi.RESET}"
    )
    return failed != 0

def main(args=[]) -> int:
    if any(cmd in args for cmd in ['help', '--help', '-h']):
        print(__doc__)
        return 0
    global wrasm, lld, qemu, pid
    wrasm = find_wrasm()
    lld = find_lld()
    qemu = find_qemu()
    pid = getpid()
    return testall()

if __name__ == "__main__":
    exit(main(argv[1:]))

