import subprocess
import glob
import os.path
import sys
import platform
import time
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

AddOption(
    '--pack-version',
    dest='pack_version',
    type='string',
    nargs=1,
    action='store',
    default='trunk',
    help='version info, ex. 1.0.0.0',
)

AddOption(
    '--date',
    dest='date',
    type='string',
    nargs=1,
    action='store',
    default=time.strftime('%Y%m%d', time.localtime(time.time())),
    help='date info, ex. 20220302 represents 2022/03/02',
)

AddOption(
    '--release',
    dest='release',
    const=True,
    action='store_const',
    default=False,
    help='compile with -O2 level',
)

def GetCommitID():
    raw_commit = subprocess.check_output(['git','rev-parse','--short','HEAD'])
    return raw_commit.strip().decode('utf-8')

prefix = GetOption('prefix')
commit_id = GetCommitID()
pack_date = GetOption('date')
version = GetOption('pack_version')

def GetPackageVersion():
    return '-'.join([version, pack_date, commit_id])

# date-commitid
def GetRevision():
    return '-'.join([pack_date, commit_id])

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

release = GetOption("release")
if plat == 'win32':
    # https://docs.microsoft.com/en-us/cpp/build/reference/mp-build-with-multiple-processes?view=msvc-140
    cppflags = ['/W3', '/Z7', '/MT']
    if release:
        cppflags += ['/O2']
    linkflags = ['/LTCG', '/IGNORE:4099']
    cppdefines = ['_WIN32', 'WIN32', 'WINNT', '_WINDOWS', 'NDEBUG', 'ENABLE_ZLIB','ENABLE_OPENSSL']
    cpppath = ['c:/developer/apr-dist/include', 'c:/developer/flow/include', 'c:/developer/libiconv-dist/include']
    syslibs = ['Advapi32.lib', 'Ws2_32.lib', 'wevtapi.lib', 'ole32.lib', 'Dbghelp.lib', 'Gdi32.lib', 'User32.lib']
    win_edition = GetOption('win_edition')
    mylibs += ['c:/developer/flow/lib/ssleay32.lib',
    'c:/developer/flow/lib/libeay32.lib', 
    'c:/developer/flow/lib/zlib.lib']
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
        cppdefines += ['_USING_V110_SDK71_', '_MBCS', '_WIN32_WINNT=0x0502', 'DISABLE_IM_MSVISTALOG']
        env['LINKFLAGS'] = ['/SUBSYSTEM:CONSOLE",5.01"']
    else:
        # MSVC_VERSION - Sets the preferred version of Microsoft Visual C/C++ to use.
        # If $MSVC_VERSION is not set, SCons will (by default) select the latest version of Visual C/C++ installed on your system.
        cppdefines += ['_WIN32_WINNT=0x0600']
else:
    env = Environment()
    cppflags = ['-g', '-fPIC']
    libpath += ['.']
    if plat == 'linux':
        # Dwarf Error: wrong version in compilation unit header (is 5, should be 2, 3, or 4)
        cppflags += ['-gdwarf-4', '-gstrict-dwarf']
    if release:
        cppflags += ['-O2']
    cppdefines = ['HAVE_CONFIG_H', '_REENTRANT', '_GNU_SOURCE', 'PIC', 'ENABLE_ARCHIVE', 'ENABLE_OPENSSL', 'ENABLE_ZLIB']
    if plat == 'aix':
        pass
        #syslibs = ['z', 'pthread', 'ssl', 'crypto']
        # On AIX 6.1, prefer system libraries if there are multiple copies of a library (for example, libiconv).
        #libpath += ['/usr/local/lib', '/usr/lib']
    elif plat == 'hpux':
        pass
        #syslibs = ['pthread', 'ssl', 'crypto', 'rt', 'z']
    else:
        #syslibs = [':libz.a.a', 'pthread', 'ssl', 'crypto', 'rt', 'dl']
        if plat == 'linux' and platform.processor() == 'i686':
            # https://stackoverflow.com/questions/22663897/unknown-type-name-off64-t
            cppdefines += ['_LARGEFILE64_SOURCE']

env['CPPFLAGS'] = cppflags
env['CPPDEFINES'] = cppdefines
env['CPPPATH'] = cpppath
env['LIBPATH'] = libpath

lib_sources = 'lumberjack.c utils.c'.split()

env.StaticLibrary('lumberjack', lib_sources)

env.Program('client', ['examples/client.c'], LIBS=['lumberjack'] + syslibs)

files += ['liblumberjack.a']

if GetOption('clean'):
    extras = ['VERSION', 'client.exp', 'client.lib', '.sconsign.dblite', 'vc140.pdb', 'client.ilk']
    for target in extras:
        env.Command(target, '', Delete('${TARGET}'))
    if os.path.exists(prefix):
        shutil.rmtree(prefix)
elif not GetOption('help'):
    includes = ['lumberjack.h', 'constant.h', 'utils.h']
    env.Install(prefix + "/include/lumberjack", includes)
    env.Install(prefix + "/lib/lumberjack", files)

