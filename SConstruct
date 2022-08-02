def getVersionFromFile():
    with open('version', 'r') as versionFile:
        version = versionFile.read()
        version = version.strip()
        print('version: ' + version)
        return version
    return 'unknown'


platform = str(ARGUMENTS.get('OS', Platform()))
print('platform: ' + platform)


envTools = []


if platform == 'win32':
    envTools.append('mingw')
    print('use mingw')


env = Environment(
    CPPPATH = [],
    CPPDEFINES = [('POTCREATOR_VERSION', getVersionFromFile())],
    LIBPATH = [],
    LIBS = [],
    tools = envTools,
    CCFLAGS = ['-pthread', '-O2', '-Wall'],
    SCONS_CXX_STANDARD = 'c++20'
)


Export('env')


SConscript('libs/SConscript', variant_dir = 'build/libs', duplicate=False)
SConscript('src/SConscript', variant_dir = 'build/potc', duplicate=False)
