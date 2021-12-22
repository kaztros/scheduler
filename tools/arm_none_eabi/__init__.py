import SCons.Tool

def phrase_args(prefix, **kwargs):
  result_list = []
  for key, value in kwargs.items():
    result_string = prefix + str(key)
    if value is not None:
      result_string += '=' + str(value)
    result_list += [result_string]
  
  return result_list


def generate(env, **kwargs):
  #
  #env['ENV']['PATH'] += ';' + 'C:\\Users\\kaz\\Development\\gcc-arm-none-eabi-9-2020-q2-update-win32\\bin'
  env['ENV']['PATH'] += ':' + '/Users/kaz/Development/gcc-arm-none-eabi-10.3-2021.10/bin'
  env['AR'] = 'arm-none-eabi-gcc-ar'
  env['AS'] = 'arm-none-eabi-gcc -x assembler-with-cpp -c'
  env['CC'] = 'arm-none-eabi-gcc'
  env['CP'] = 'arm-none-eabi-objcopy'
  env['CXX'] = 'arm-none-eabi-g++'
  env['LINK'] = 'arm-none-eabi-g++'
  env['RANLIB'] = 'arm-none-eabi-gcc-ranlib'
  env['SZ'] = 'arm-none-eabi-size'
  #
  env['OBJPREFIX'] = ''
  env['OBJSUFFIX'] = '.o'
  env['SHOBJPREFIX'] = '$OBJPREFIX'
  env['SHOBJSUFFIX'] = '$OBJSUFFIX'
  env['PROGPREFIX'] = ''
  env['PROGSUFFIX'] = ''
  env['LIBPREFIX'] = 'lib'
  env['LIBSUFFIX'] = '.a'
  env['SHLIBPREFIX'] = '$LIBPREFIX'
  env['SHLIBSUFFIX'] = '.so'
  env['LIBPREFIXES'] = [ '$LIBPREFIX' ]
  env['LIBSUFFIXES'] = [ '$LIBSUFFIX', '$SHLIBSUFFIX' ]
  #
  env["TARGET_ARCH"]="arm-none-eabi"
  #
  SCons.Tool.createObjBuilders(env)
  SCons.Tool.createStaticLibBuilder(env)
  SCons.Tool.createProgBuilder(env)


def exists(env):
#  return os.path.exists('C:\\Users\\kaz\\ac6\\plugins\\fr.ac6.mcu.externaltools.arm-none.win32_1.17.0.201812190825\\tools\\compiler\\bin')
  return os.path.exists('/home/kaz/Development/gcc-arm-none-eabi-10.3-2021.10/bin')


