project('ubicast-video-server', 'c',
  license: 'LGPL-2.1-or-later',
  version: '1.0')

gst_min_ver = '>=1.18'

gst_dep = dependency('gstreamer-1.0',
  version: gst_min_ver)
gstrtsp_dep = dependency('gstreamer-rtsp-1.0',
  version: gst_min_ver)
gstrtspserver_dep = dependency('gstreamer-rtsp-server-1.0',
  version: gst_min_ver)

glib_dep = dependency('glib-2.0',
  version : '>=2.66')

vls_sources = [
  'src/vls.c',
  'src/vls_rtsp.c',
]

vls_deps = [
  gst_dep,
  gstrtsp_dep,
  gstrtspserver_dep,
  glib_dep
]

bindir = join_paths(get_option('prefix'), get_option('bindir'))
libdir = join_paths(get_option('prefix'), get_option('libdir'))

service_file = configure_file(
  input: 'vls@.service.in',
  output: 'vls@.service',
  configuration:
	{'install_path': bindir}
)

install_data(service_file,
  install_dir: join_paths(libdir, 'systemd', 'system')
)
  
executable('vls', vls_sources,
  dependencies: vls_deps,
  install: true)
