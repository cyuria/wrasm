tests = [
    'find_symbol_or_immediate.c',
    'get_register_id.c',
    'parse_form.c',
    'parse_args.c',
    'form_base.c',
    'form_atomic.c',
    'form_csr.c',
]

foreach test : tests
    e = executable(
        'test_' + test,
        test,
        sources,
        build_by_default: false,
        include_directories: [headers],
        dependencies: [argtable3_dep],
        c_args: [
            '-D_CRT_SECURE_NO_WARNINGS',
        ],
        native: not meson.can_run_host_binaries(),
    )
    test('c unit test ' + test, e)
endforeach
