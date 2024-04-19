project('wrasm', 'c')

threads = dependency('threads')
argtable3 = subproject('argtable3').get_variable('argtable3_dep')

headers = include_directories('h')
libincludes = include_directories('subprojects')
subdir('src')

executable(
  'wrasm', sources,
  include_directories : [ headers, libincludes ],
  dependencies : [ threads, argtable3 ],
  install : true
)
