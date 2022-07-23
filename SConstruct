env=Environment(
    CPPPATH='include;libs/json/single_include;libs/rxterm/include;libs/argh',
    # CPPDEFINES=['foo'],
    # LIBS=['bar'],
    CCFLAGS=['-pthread', '-O2', '-Wall'],
    tools = ["mingw"],
    SCONS_CXX_STANDARD='c++20')

env.Program('potcreator', Glob('src/*.cpp'))

#Program('potcreator', Glob('src/*.cpp'))
