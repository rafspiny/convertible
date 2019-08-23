project('convertible', 'c',
        version: '0.1',
        license: 'MPL 2',
        default_options: [ 'c_std=gnu99', 'warning_level=2' ],
        meson_version: '>= 0.47.0')
pkgconfig = import('pkgconfig')
cc = meson.get_compiler('c')

dep_e = dependency('enlightenment')
dep_edje = dependency('edje' , required: true)
deps = [
        dep_e,
        dependency('elementary'),
        dependency('ecore'),
        dependency('eldbus')
]
release = dep_e.get_pkgconfig_variable('release')
host_os = host_machine.system()
if host_os == 'linux'
        if cc.has_header_symbol('features.h', '__UCLIBC__')
                host_os = 'linux-uclibc'
        elif cc.has_header_symbol('features.h', '__dietlibc__')
                host_os = 'linux-dietlibc'
        else
                host_os = 'linux-gnu'
        endif
endif
module_arch = '@0@-@1@-@2@'.format(host_os, host_machine.cpu_family(), release)
dir_prefix = get_option('prefix')
dir_lib = join_paths(dir_prefix, get_option('libdir'))
dir_gadgets = join_paths([dir_lib, 'enlightenment/modules', 'convertible'])
build_files = [
        'src/mod.c',
        'src/dbus_acceleration.h',
        'src/dbus_acceleration.c',
        'src/e-gadget-convertible.h',
        'src/e-gadget-convertible.c',
        'src/accelerometer-orientation.h',
        'src/convertible_logging.h',
        'src/convertible.h',
        'src/e_mod_config.h',
        'src/e_mod_config.c'
]

inc = include_directories('.', '../../')

# subdir('theme')

shared_module('module', build_files,
        include_directories: inc,
        name_prefix        : '',
        dependencies       : [ dep_e, deps ],
        install_dir        : join_paths([dir_gadgets, module_arch]),
        install            : true,
        link_args          : '-Wl,--unresolved-symbols=ignore-all'
       )

desktop_data = configuration_data()
        desktop_data.set('GADGET_DIR', dir_gadgets)

configure_file(input: 'convertible.desktop.in',
        output: 'module.desktop',
        install: true,
        install_dir: dir_gadgets,
        configuration: desktop_data)

configure_file(input: 'theme/convertible.edj',
        output: 'e-module-convertible.edj',
        install: true,
        install_dir: dir_gadgets,
        copy: true)

configure_file(input: 'theme/convertible-icon.edj',
        output: 'convertible-icon.edj',
        install: true,
        install_dir: dir_gadgets,
        copy: true)
