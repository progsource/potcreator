env=Environment(
    CPPPATH=[],
    # CPPDEFINES=['foo'],
    # LIBS=['bar'],
    CCFLAGS=['-pthread', '-O2', '-Wall'],
    tools = ["mingw"],
    SCONS_CXX_STANDARD='c++20')
Export('env')

SConscript('libs/SConscript', variant_dir='build/libs', duplicate=False)
SConscript('src/SConscript', variant_dir='build/potc', duplicate=False)
