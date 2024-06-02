project('argtable3', 'c')
cc = meson.get_compiler('c')
libm = cc.find_library('m', required: false)
headers = include_directories('include')
argtable3 = static_library(
    'argtable3',
    'src/argtable3.c',
    include_directories: [headers],
    dependencies: [libm],
)
argtable3_dep = declare_dependency(link_with: argtable3, include_directories: headers)
