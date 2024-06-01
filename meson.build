project(
    'wrasm',
    'c',
    default_options: [
        'c_std=c17',
        'warning_level=3',
        'b_lto=true',
        'b_pie=true',
    ],
    version: '0.0.1-alpha',
)

argtable3 = subproject(
    'argtable3',
    required: true,
)
argtable3_dep = argtable3.get_variable('argtable3_dep')

headers = include_directories('h')

sources = files(
    'src/args.c',
    'src/bytecode.c',
    'src/debug.c',
    'src/directives.c',
    'src/elf/def.c',
    'src/elf/output.c',
    'src/form/generic.c',
    'src/form/instructions.c',
    'src/form/rv32a.c',
    'src/form/rv32i.c',
    'src/form/rv64a.c',
    'src/form/rv64i.c',
    'src/generation.c',
    'src/parse.c',
    'src/registers.c',
    'src/stringutil.c',
    'src/symbols.c',
    'src/xmalloc.c',
)

executable(
    'wrasm',
    'src/main.c',
    sources,
    include_directories: [headers],
    dependencies: [argtable3_dep],
    c_args: [
        '-D_CRT_SECURE_NO_WARNINGS',
    ],
    install: true,
)

subdir('test/unit')
