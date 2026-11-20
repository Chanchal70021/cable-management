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

conf_data = configuration_data()
conf_data.set_quoted(
    'HUYGENS_4N_CABLE_JSON_PATH',
    get_option('HUYGENS_4N_CABLE_JSON_PATH'),
)
configure_file(output: 'config.h', configuration: conf_data)

phosphor_logging_dep = dependency('phosphor-logging', required: true)
sdbusplus_dep = dependency('sdbusplus', required: true)
boost_dep = dependency('boost', required: true)
nlohmann_json_dep = dependency('nlohmann_json', required: true)
libgpiod_dep = dependency('libgpiodcxx', required: true)

cable_manager_sources = [
    'src/cable_manager_main.cpp',
    'src/manager.cpp',
    'src/handler.cpp',
    'src/cdfp_handler.cpp',
    'src/config_manager.cpp',
]

executable(
    'cable-manager',
    cable_manager_sources,
    include_directories: include_directories('.', 'include'),
    dependencies: [
        phosphor_logging_dep,
        sdbusplus_dep,
        boost_dep,
        nlohmann_json_dep,
        libgpiod_dep,
    ],
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
install_data(
    'configs/huygens_4N.json',
    install_dir: get_option('datadir') / 'cable-manager',
)
