project('y-data','c', version: '0.0.0')

libgio_dep = dependency('gio-2.0', version: '>= 2.44')
libgobj_dep = dependency('gobject-2.0', version: '>= 2.44')
fftw_dep = dependency('fftw3', version: '>=3.2')

comp = meson.get_compiler('c')
hdf5 = [comp.find_library('hdf5_hl', required: false),comp.find_library('hdf5', required: false)]
libm = comp.find_library('m', required: false)

conf = configuration_data()
conf.set_quoted('PACKAGE_NAME', 'y-data')

subdir('docs')
subdir('src')
subdir('test')

