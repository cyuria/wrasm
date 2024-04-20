
bash = find_program('bash')
llvmcompare = find_program('llvmcompare.sh')

test('w2-llvm', bash, args : [
  llvmcompare.full_path(),
  wrasm.full_path(),
  meson.source_root() / 'test' / 'w2.S'
], is_parallel : false)
