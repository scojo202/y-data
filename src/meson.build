src_header_subdir = join_paths('y-data','src')

src_public_headers = []
src_public_sources = []

src_public_headers += [
  'y-data-class.h',
  'y-data.h',
  'y-data-simple.h',
  'y-linear-range.h',
  'y-scalar-property.h',
  'y-vector-ring.h',
  'y-data-derived.h',
  'y-operation.h',
  'y-slice-operation.h',
  'y-hdf.h',
  'y-data-tcp.h',
  'y-fft-operation.h'
]

src_public_sources += [
  'y-data.c',
  'y-data-simple.c',
  'y-linear-range.c',
  'y-scalar-property.c',
  'y-vector-ring.c',
  'y-data-derived.c',
  'y-operation.c',
  'y-slice-operation.c',
  'y-hdf.c',
  'y-data-tcp.c',
  'y-fft-operation.c'
]

install_headers(src_public_headers,subdir: 'libydata-0.0')

ydata_deps = [libgobj_dep, libgio_dep, fftw_dep, libm, hdf5]

libydata = shared_library('ydata-0.0',src_public_sources, dependencies: ydata_deps, install: true, install_dir: get_option('libdir'))

libydata_dep = declare_dependency(dependencies: ydata_deps, link_with: libydata, include_directories: include_directories('.'),)

pkgg = import('pkgconfig')

pkgg.generate(
    libraries: [libydata],
      subdirs: [ 'libydata-0.0'.format(meson.project_version()) ],
      version: meson.project_version(),
         name: 'y-data',
     filebase: 'y-data-0.0',
  description: 'YData',
     requires: [ 'glib-2.0', 'gobject-2.0', 'gio-2.0', 'fftw3'],
  install_dir: join_paths(get_option('libdir'), 'pkgconfig'),
)
