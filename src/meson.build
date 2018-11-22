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
  'y-fft-operation.h',
  'y-simple-operation.h',
  'y-subset-operation.h'
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
  'y-fft-operation.c',
  'y-simple-operation.c',
  'y-subset-operation.c'
]

install_headers(src_public_headers,subdir: 'libydata-0.0')

ydata_deps = [libgobj_dep, libgio_dep, fftw_dep, libm, hdf5]

libydata = shared_library('ydata-0.0',src_public_sources, dependencies: ydata_deps, install: true, install_dir: get_option('libdir'))

libydata_dep = declare_dependency(dependencies: ydata_deps, link_with: libydata, include_directories: include_directories('.'),)

ydata_dep_sources = []

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

# Introspection
if build_gir
  gir_extra_args = [
    '--quiet',
    '--warn-all',
  ]

  ydata_gir = gnome.generate_gir(libydata,
                                    sources: src_public_headers + src_public_sources,
                                    namespace: 'YData',
                                    nsversion: '0.0',
                                    identifier_prefix: 'Y',
                                    symbol_prefix: 'y',
                                    export_packages: 'ydata',
                                    includes: [ 'GObject-2.0', 'Gio-2.0' ],
                                    install: true,
                                    extra_args: gir_extra_args)
  ydata_dep_sources += ydata_gir
endif
