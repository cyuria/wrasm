
functiontests = [
  'find_symbol_or_immediate.c',
  'get_register_id.c',
]

systemtests = [
  'w2.S',
]

bash = find_program('bash')
llvmcompare = find_program('system' / 'llvmcompare.sh')

srcincludeargs = []
foreach file : sources
  srcincludeargs += '-include' + file
endforeach

root = include_directories('../')

foreach function : functiontests
  e = executable(
    'test_' + function,
    'function' / function,
    include_directories : [ headers, root ],
    dependencies : [ threads, argtable3_dep ],
    c_args : srcincludeargs,
  )
  test('c function ' + function, e)
endforeach

foreach file : systemtests
  test('llvm compare ' + file, bash, args : [
    llvmcompare.full_path(),
    wrasm.full_path(),
    meson.source_root() / 'test' / 'system' / file
  ])
endforeach
