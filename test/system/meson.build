
tests = [
  'helloworld',
]

python = find_program(['python3', 'python', 'py'])
qemu = find_program(['qemu-riscv64', 'qemu-riscv64-static'])
compareoutput = find_program('compareoutput.py')

foreach test : tests
  test('expected output ' + test, python, args : [
    compareoutput.full_path(),
    qemu.full_path(),
    wrasm.full_path(),
    meson.source_root() / 'test' / 'system' / test
  ])
endforeach
