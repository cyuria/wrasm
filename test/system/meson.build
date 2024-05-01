
tests = [
  'helloworld',
]

python = find_program(['python3', 'python', 'py'])
compareoutput = find_program('compareoutput.py')

foreach test : tests
  test('expected output ' + test, python, args : [
    compareoutput.full_path(),
    wrasm.full_path(),
    meson.source_root() / 'test' / 'system' / test
  ])
endforeach
