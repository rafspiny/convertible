##### edje_cc binary compiler tool
edje_cmd = get_option('edje-cc')
if edje_cmd == ''
  edje_cmd = join_paths(dep_edje.get_pkgconfig_variable('prefix'), 'bin', 'edje_cc')
endif

edc_src = [
    'convertible', 'convertible-icon'
]

foreach edc_file : edc_src
  custom_target(edc_file + '.edj',
                  input                 : edc_file + '.edc',
                  output                : edc_file + '.edj',
                  command               : [ edje_cmd, meson.current_source_dir() + '/' + edc_file + '.edc' ],
                  install_dir           : 'theme',
                  install_mode          : 'rw-r-xr-x',
                  install               : true
  )
endforeach
