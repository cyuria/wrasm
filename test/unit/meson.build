tests = [
    'find_symbol_or_immediate.c',
    'get_register_id.c',
    'parsers_gen_math.c',
]

compiler = meson.get_compiler('c')

includeflag = '-include'

if (compiler.get_id() == 'msvc')
    includeflag = '/FI'
endif

root = include_directories('../../')

foreach test : tests
    e = executable(
        'test_' + test,
        test,
        sources,
        include_directories: [headers, root],
        dependencies: [threads, argtable3_dep],
        c_args: [
            '-D_CRT_SECURE_NO_WARNINGS',
        ],
    )
    test('c unit test ' + test, e)
endforeach
