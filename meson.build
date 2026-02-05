project('gradcasino', 'cpp',
    version: '0.1.0',
    license: 'MIT',
    meson_version: '>=1.2.0',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
        'werror=true',
        'optimization=2',
        'b_ndebug=if-release',
    ]
)

cpp = meson.get_compiler('cpp')

extra_args = []
if cpp.get_id() == 'gcc' or cpp.get_id() == 'clang'
    extra_args += [
        '-Wconversion',
        '-Wshadow',
        '-Wnon-virtual-dtor',
        '-Wold-style-cast',
        '-Wcast-align',
        '-Wunused',
        '-Woverloaded-virtual',
        '-Wpedantic',
    ]
endif

llvm_dep = dependency('llvm',
    version: '>=15.0',
    modules: ['core', 'orcjit', 'native'],
    required: true,
)

gtest_dep = dependency('gtest',
    required: true,
    fallback: ['gtest', 'gtest_dep'],
)

gtest_main_dep = dependency('gtest_main',
    required: true,
    fallback: ['gtest', 'gtest_main_dep'],
)

inc = include_directories('include')

gradcasino_sources = files(
    'src/engine/engine.cpp',
    'src/engine/graph.cpp',
    'src/engine/double.cpp',
    'src/backend/llvm/codegen.cpp',
    'src/backend/llvm/jit.cpp',
)

gradcasino_lib = library('gradcasino',
    gradcasino_sources,
    include_directories: inc,
    dependencies: [llvm_dep],
    cpp_args: extra_args,
    install: true,
)

gradcasino_dep = declare_dependency(
    include_directories: inc,
    link_with: gradcasino_lib,
)

install_subdir('include/gradcasino', install_dir: get_option('includedir'))

subdir('tests')
