
tests = [
  'find_symbol_or_immediate.c',
  'get_register_id.c',
]

compiler = meson.get_compiler('c')

includeflag = '-include'

if (compiler.get_id() == 'msvc')
  includeflag = '/FI'
endif

srcincludeargs = []
foreach file : sources
  srcincludeargs += includeflag + file
endforeach

root = include_directories('../../')

foreach test : tests
  e = executable(
    'test_' + test,
    test,
    include_directories : [ headers, root ],
    dependencies : [ threads, argtable3_dep ],
    c_args : [ srcincludeargs, '-D_CRT_SECURE_NO_WARNINGS', ],
  )
  test('c unit test ' + test, e)
endforeach
