project(
    'cable-manager',
    'cpp',
    version: '1.0',
    meson_version: '>=1.1.1',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++23',
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
        'b_lto=true',
    ],
)

cxx = meson.get_compiler('cpp')
phosphor_logging_dep = dependency('phosphor-logging', required: true)
sdbusplus_dep = dependency('sdbusplus', required: true)
boost_dep = dependency('boost', required: true)
libgpiod_dep = dependency('libgpiodcxx', required: true)

cable_manager_sources = ['src/cable_manager_main.cpp', 'src/manager.cpp']

executable(
    'cable-manager',
    cable_manager_sources,
    include_directories: include_directories('include'),
    dependencies: [phosphor_logging_dep, sdbusplus_dep, boost_dep, libgpiod_dep],
    install: true,
    install_dir: get_option('bindir'),
)

# Install systemd service file
systemd_system_unit_dir = dependency('systemd').get_variable(
    'systemdsystemunitdir',
)
install_data(
    'service_files/cable-manager.service',
    install_dir: systemd_system_unit_dir,
)
