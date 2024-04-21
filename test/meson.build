
functiontests = [
  'find_symbol_or_immediate.c',
  'get_register_id.c',
]

systemtests = [
  'w2.S',
]

python = find_program(['python3', 'python', 'py'])
llvmcompare = find_program('system' / 'llvmcompare.py')

compiler = meson.get_compiler('c')

includeflag = '-include'

if (compiler.get_id() == 'msvc')
  includeflag = '/FI'
endif

srcincludeargs = []
foreach file : sources
  srcincludeargs += includeflag + file
endforeach

root = include_directories('../')

foreach function : functiontests
  e = executable(
    'test_' + function,
    'function' / function,
    include_directories : [ headers, root ],
    dependencies : [ threads, argtable3_dep ],
    c_args : [ srcincludeargs, '-D_CRT_SECURE_NO_WARNINGS', ],
  )
  test('c function ' + function, e)
endforeach

foreach file : systemtests
  test('llvm compare ' + file, python, args : [
    llvmcompare.full_path(),
    wrasm.full_path(),
    meson.source_root() / 'test' / 'system' / file
  ])
endforeach
