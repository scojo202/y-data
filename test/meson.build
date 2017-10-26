testdeps = [libgio_dep, libgobj_dep]

test_cflags = [
  '-I'+meson.source_root()+'/src',
]

testdata = executable('testdata',
  'testdata.c',
  c_args : test_cflags,
  link_args : ['-lm'],
  dependencies: [
    testdeps,
    hdf5,
    libydata_dep
  ],
)

meson.source_root()+'/src'
