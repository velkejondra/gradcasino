test_sources = files(
    'test_double.cpp',
    'test_graph.cpp',
    'test_compile.cpp',
    'test_where.cpp',
    'test_loop.cpp',
    'test_interpreter.cpp',
)

gradcasino_tests = executable('gradcasino_tests',
    test_sources,
    dependencies: [gradcasino_dep, gtest_dep, gtest_main_dep],
)

test('unit_tests', gradcasino_tests)
