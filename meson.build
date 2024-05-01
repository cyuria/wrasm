project('wrasm', 'c', default_options : [
  'c_std=c17',
  'warning_level=3',
  'werror=true',
  'b_lto=true',
], version : '0.0.1-alpha')

threads = dependency('threads')
argtable3 = subproject('argtable3', default_options : [
  'argtable3:werror=false',
], required : true)
argtable3_dep = argtable3.get_variable('argtable3_dep')

headers = include_directories('h')

sources = [
  'src/args.c',
  'src/asm.c',
  'src/debug.c',
  'src/directives.c',
  'src/elf/def.c',
  'src/elf/output.c',
  'src/generation.c',
  'src/instructions.c',
  'src/parsers.c',
  'src/registers.c',
  'src/stringutil.c',
  'src/symbols.c',
  'src/xmalloc.c',
]

wrasm = executable(
  'wrasm', [ 'src/main.c', files(sources) ],
  include_directories : [ headers ],
  dependencies : [ threads, argtable3_dep ],
  c_args : [ '-D_CRT_SECURE_NO_WARNINGS', ],
  install : true
)

subdir('test/unit')
subdir('test/system')
