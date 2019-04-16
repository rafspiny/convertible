project('convertible', 'c',
        version: '0.1',
        license: 'MPL 2',
        default_options: [ 'c_std=gnu99', 'warning_level=2' ],
        meson_version: '>= 0.40.0')
pkgconfig = import('pkgconfig')
cc = meson.get_compiler('c')

dep_e = dependency('enlightenment')
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
dir_gadgets = join_paths([dir_lib, 'enlightenment/gadgets', module_arch])
build_files = [
        'src/main.c'
]

inc = include_directories('.', '../../')

executable('e_gadget_convertible',
        build_files,
        dependencies: deps,
        c_args : ['-fPIE', '-fPIC'],
        link_args : ['-rdynamic', '-pie'],
        install_dir: join_paths([dir_gadgets, 'e_gadget_convertible']),
        install: true)

desktop_data = configuration_data()
        desktop_data.set('GADGET_DIR', dir_gadgets)

configure_file(input: 'helloworld.desktop.in',
        output: 'e_convertible.desktop',
        install: true,
        install_dir: join_paths([dir_gadgets, 'e_gadget_convertible']),
        configuration: desktop_data)