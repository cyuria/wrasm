
llvmcompare = find_program('llvmcompare.sh')

test('w2-llvm', llvmcompare, args : [
  wrasm.full_path(),
  meson.source_root() / 'test' / 'w2.S'
], is_parallel : false)
