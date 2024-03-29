import os.path
import sys
import platform
import shutil

# A static library is built per each one of the following directory list.

# Normallize according to https://docs.python.org/3/library/sys.html#sys.platform
plat = sys.platform
if plat.startswith('linux'):
    plat = 'linux'
elif plat.startswith('aix'):
    plat = 'aix'
elif plat in ['win32', 'cygwin']:
    plat = 'win32'
elif plat.startswith('hp-ux'):
    plat = 'hpux'
else:
    print('unsupported platform: %s'% plat)
    sys.exit(-1)


def fitPlat(fn):
    base = os.path.splitext(os.path.basename(fn))[0]
    fields = base.split('-')
    if len(fields) <= 1 or fields[1] not in ['unix', 'win32'] or fields[1] == plat or (fields[1] == 'unix' and plat in ['linux', 'aix', 'hpux']):
        return True
    return False


if plat == 'win32':
    AddOption(
        '--win-edition',
        dest='win_edition',
        type='string',
        nargs=1,
        action='store',
        default='2008',
        help='Target windows platform. Set to 2003 to disable MSVISTALOG.',
    )

AddOption(
    '--prefix',
    dest='prefix',
    type='string',
    nargs=1,
    action='store',
    metavar='DIR',
    default='./build/',
    help='installation prefix',
)

prefix = GetOption('prefix')

# https://www.gnu.org/software/autoconf/manual/autoconf-2.64/html_node/Standard-Symbols.html#Standard-Symbols
# 5.1.1 Standard Symbols
# All the generic macros that AC_DEFINE a symbol as a result of their test transform their argument values to a standard alphabet. First, argument is converted to upper case and any asterisks (‘*’) are each converted to ‘P’. Any remaining characters that are not alphanumeric are converted to underscores.
def ToMacro(item):
    macro = 'HAVE_'
    for c in item:
        if c == '*':
            macro += 'P'
        elif not c.isalnum():
            macro += '_'
        else:
            macro += c.upper()
    return macro

# https://github.com/SCons/scons/issues/1065
# https://www.gnu.org/software/autoconf/manual/autoconf-2.64/html_node/Generic-Structures.html#Generic-Structures
# Refers to https://github.com/SCons/scons/blob/master/SCons/Conftest.py#355, CheckType.
def CheckMember(context, aggregate_member, header = None, language = None):
    """
    Configure check for a C or C++ member "aggregate_member".
    Optional "header" can be defined to include a header file.
    "language" should be "C" or "C++" and is used to select the compiler.
    Default is "C".
    Note that this uses the current value of compiler and linker flags, make
    sure $CFLAGS, $CPPFLAGS and $LIBS are set correctly.
    Returns:
        status : int
            0 if the check failed, 1 if succeeded."""
    context.Display("Checking for member %s... " % (aggregate_member))
    suffix = _lang2suffix(language)
    if suffix is None:
        context.Result(0)
        return 0
    fields = aggregate_member.split('.')
    if len(fields) != 2:
        context.Result(0)
        return 0
    aggregate, member = fields[0], fields[1]
    if context.headerfilename:
        includetext = '#include "%s"' % context.headerfilename
    else:
        includetext = ''
    if not header:
        header = ''
    text = '''
%(include)s
%(header)s

int main(void) {
  if (sizeof ((%(aggregate)s *) 0)->%(member)s)
    return 0;
}''' % { 'include': includetext,
         'header': header,
         'aggregate': aggregate,
         'member':member }

    ret = context.TryLink(text, suffix)
    #print(text, suffix, ret)
    context.Result(ret)
    return ret


def _lang2suffix(lang):
    """
    Convert a language name to a suffix.
    When "lang" is empty or None C is assumed.
    """
    if not lang or lang in ["C", "c"]:
        return '.c'
    if lang in ["c++", "C++", "cpp", "CXX", "cxx"]:
        return '.cpp'
    return None

cppflags = list()
cppdefines = list()
cpppath = list()
libpath = list()
syslibs = list()
mylibs = list()
files = list()

if plat == 'win32':
    # https://docs.microsoft.com/en-us/cpp/build/reference/mp-build-with-multiple-processes?view=msvc-140
    cppflags = ['/W3', '/Z7', '/FD', '/MP', '/FS']
    linkflags = ['/LTCG', '/IGNORE:4099']
    cppdefines = ['_WIN32', 'WIN32', 'WINNT', '_WINDOWS', 'NDEBUG', '_CRT_SECURE_NO_WARNINGS', '_WINSOCK_DEPRECATED_NO_WARNINGS', 'HAVE_ZLIB_H','HAVE_SSL_H']
    cpppath = ['c:/developer/flow/include']
    libpath = ['.', 'c:/developer/flow/lib']
    syslibs = ['kernel32.lib', 'user32.lib', 'advapi32.lib', 'ws2_32.lib', 
               'wsock32.lib', 'netapi32.lib', 'Dbghelp.lib', 'version.lib', 
               'shell32.lib', 'wevtapi.lib', 'ole32.lib', 'Gdi32.lib',
               'ssleay32.lib', 'libeay32.lib', 'zlib.lib']
    exe_suffix = ['client', 'event', 'boot', 'client_cpp']
    files = [item if item not in exe_suffix else item + '.exe' for item in files]
    win_edition = GetOption('win_edition')
    VC_CRT_DIST="C:/Program Files (x86)/Microsoft Visual Studio 8/VC/redist/x86/Microsoft.VC80.CRT/"
    VC_DEBUG_CRT_DIST="C:/Program Files (x86)/Microsoft Visual Studio 8/VC/redist/Debug_NonRedist/x86/Microsoft.VC80.DebugCRT/"
    VS2005_DIST = 'C:/developer/vs2005/'
    if os.path.exists(VS2005_DIST):
        VC_CRT_DIST = VS2005_DIST
        VC_DEBUG_CRT_DIST = VS2005_DIST
    files += [
        VC_CRT_DIST + 'msvcr80.dll', 
        VC_DEBUG_CRT_DIST + 'msvcr80d.dll',
        VC_CRT_DIST + 'Microsoft.VC80.CRT.manifest',
        VC_DEBUG_CRT_DIST + 'Microsoft.VC80.DebugCRT.manifest',
    ]
    env = Environment(TARGET_ARCH='x86')
    env.Append(LINKFLAGS=linkflags)
    if win_edition == '2003':
        cppdefines += ['_USING_V110_SDK71_', '_MBCS', '_WIN32_WINNT=0x0502']
        env['LINKFLAGS'] = ['/SUBSYSTEM:CONSOLE",5.01"']
    else:
        # MSVC_VERSION - Sets the preferred version of Microsoft Visual C/C++ to use.
        # If $MSVC_VERSION is not set, SCons will (by default) select the latest version of Visual C/C++ installed on your system.
        cppdefines += ['_WIN32_WINNT=0x0600']
else:
    env = Environment()
    cppdefines = ['HAVE_CONFIG_H', '_REENTRANT', '_GNU_SOURCE', 'PIC', 'HAVE_SSL_H', 'HAVE_ZLIB_H']
    cpppath = ['/usr/local/eoitek/include']
    libpath = ['.', '/usr/local/eoitek/lib']
    cppflags = ['-g', '-fPIC']
    
    if plat == 'linux':
        # Dwarf Error: wrong version in compilation unit header (is 5, should be 2, 3, or 4)
        cppflags += ['-gdwarf-4', '-gstrict-dwarf']
    if plat == 'aix':
        # https://blog.csdn.net/in2000/article/details/6077968 errno on aix is not thread safe
        cppdefines += ['_THREAD_SAFE']
        syslibs = ['z', 'ssl', 'crypto', 'rt', 'dl', 'pthread']
    elif plat == 'hpux':
        syslibs = ['z', 'ssl', 'crypto', 'rt', 'dl', 'pthread']
    else:
        syslibs = [':libz.a.a', 'ssl', 'crypto', 'rt', 'dl', 'pthread']
        if plat == 'linux' and platform.processor() == 'i686':
            # https://stackoverflow.com/questions/22663897/unknown-type-name-off64-t
            cppdefines += ['_LARGEFILE64_SOURCE']

env['CPPFLAGS'] = cppflags
env['CPPDEFINES'] = cppdefines
env['CPPPATH'] = cpppath
env['LIBPATH'] = libpath

lib_sources = 'lumberjack.c utils.c buf.c'.split()

env.StaticLibrary('lumberjack', lib_sources)

env.Program('client', ['examples/client.c'], LIBS=['lumberjack'] + syslibs)
env.Program('event', ['examples/event.c'], LIBS=['lumberjack'] + syslibs)
env.Program('boot', ['examples/boot.c'], LIBS=['lumberjack'] + syslibs)
# if plat != 'hpux':
#     cppflags += ['-std=c++11', '-pthread']
#     linkflags = ['-static-libgcc', '-static-libstdc++']
#     env.Program('client-cpp', ['examples/client_thread.cpp'], LIBS=['lumberjack'] + syslibs, LINKFLAGS=linkflags)
if plat == 'win32':
    files += ['lumberjack.lib']
else:
    files += ['liblumberjack.a']

if GetOption('clean'):
    if os.path.exists(prefix):
        shutil.rmtree(prefix)
elif not GetOption('help'):
    includes = ['lumberjack.h', 'constant.h', 'utils.h']
    env.Install(prefix + "/include/lumberjack", includes)
    env.Install(prefix + "/lib/lumberjack", files)

