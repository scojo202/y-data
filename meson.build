project('y-data','c')

libgio_dep = dependency('gio-2.0', version: '>= 2.44')
libgobj_dep = dependency('gobject-2.0', version: '>= 2.44')

comp = meson.get_compiler('c')
hdf5 = [comp.find_library('hdf5_hl', required: false),comp.find_library('hdf5', required: false)]

conf = configuration_data()
conf.set_quoted('PACKAGE_NAME', 'y-data')

subdir('docs')
subdir('src')
subdir('test')

