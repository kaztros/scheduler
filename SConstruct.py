from os.path import relpath

env = Environment(tools=['link', 'as', 'ar', 'gcc', 'g++', 'arm_none_eabi.cortex_m4.STM32L412KB'], toolpath=['tools'])

# print(env.Dump())

# The ARM-M4 + STM32 platform headers are kind of a mess.  Fixing their
# organization, e.g. breaking circular dependencies, requires a re-write with
# template usage, or intentionally breaking compile-time optimizations.
# 
# work-around: add this include path for every compile.
env.Append(CPPPATH=['#Core/Inc'])


# TODO: Make a function call to handle sub-sconscripts.
env.Append(CXXFLAGS=['-std=c++17', '-fno-rtti', '-fno-use-cxa-atexit'])
env.Append(CXXFLAGS=['-fno-exceptions', '-fno-non-call-exceptions'])
env.Append(CXXFLAGS=['-fno-common', '-ffreestanding'])
env.Append(CXXFLAGS=['-static-libstdc++'])
env.Append(CXXFLAGS=['-Wno-pmf-conversions'])

OptimizationLevel = ['-Og', '-gdwarf-3']
#OptimizationLevel = ['-O3', '-flto']
env.Append(CXXFLAGS=OptimizationLevel, CFLAGS=OptimizationLevel, LINKFLAGS=OptimizationLevel)

env.Append(LINKFLAGS=['-specs=nano.specs', '-specs=nosys.specs'])

subproject_dirs = [ 'Core'
                  , 'Drivers/STM32L4xx_HAL_Driver'
                  ]

libs = []
for dir in subproject_dirs:
  libs += [ SConscript(dir + '/SConscript.py', exports='env', variant_dir='build/'+dir, duplicate=0)]


env.Append(LIBS=['stdc++'])

pwm_divider_debuggable = env.Program(target='#build/PWMDivider.debug', source=libs)
Depends(pwm_divider_debuggable, '#STM32L412KBUx_FLASH.ld')
pwm_divider_stripped = env.Command('#build/PWMDivider.stripped', pwm_divider_debuggable, 'arm-none-eabi-strip --strip-all -o build/PWMDivider.stripped build/PWMDivider.debug' )
Depends(pwm_divider_stripped, pwm_divider_debuggable)

# This was suggested by STM32's default path.  But I hate it.


# TODO: scons + boost, with a configurable toolchain:

"""
boost_dirs = [ 'accumulators'
  , 'algorithm'
  , 'align'
  , 'any'
  , 'array'
  , 'asio'
  , 'assert'
  , 'assign'
  , 'atomic'
  , 'beast'
  , 'bimap'
  , 'bind'
  , 'callable_traits'
  , 'chrono'
  , 'circular_buffer'
  , 'compatibility'
  , 'compute'
  , 'concept_check'
  , 'config'
  , 'container'
  , 'container_hash'
  , 'context'
  , 'contract'
  , 'conversion'
  , 'convert'
  , 'core'
  , 'coroutine'
  , 'coroutine2'
  , 'crc'
  , 'date_time'
  , 'detail'
  , 'dll'
  , 'dynamic_bitset'
  , 'endian'
  , 'exception'
  , 'fiber'
  , 'filesystem'
  , 'flyweight'
  , 'foreach'
  , 'format'
  , 'function'
  , 'function_types'
  , 'functional'
  , 'fusion'
  , 'geometry'
  , 'gil'
  , 'graph'
  , 'graph_parallel'
  , 'hana'
  , 'headers'
  , 'heap'
  , 'histogram'
  , 'hof'
  , 'icl'
  , 'integer'
  , 'interprocess'
  , 'intrusive'
  , 'io'
  , 'iostreams'
  , 'iterator'
  , 'lambda'
  , 'lexical_cast'
  , 'local_function'
  , 'locale'
  , 'lockfree'
  , 'log'
  , 'logic'
  , 'math'
  , 'metaparse'
  , 'move'
  , 'mp11'
  , 'mpi'
  , 'mpl'
  , 'msm'
  , 'multi_array'
  , 'multi_index'
  , 'multiprecision'
  , 'nowide'
  , 'numeric'
  , 'optional'
  , 'outcome'
  , 'parameter'
  , 'parameter_python'
  , 'phoenix'
  , 'poly_collection'
  , 'polygon'
  , 'pool'
  , 'predef'
  , 'preprocessor'
  , 'process'
  , 'program_options'
  , 'property_map'
  , 'property_tree'
  , 'proto'
  , 'ptr_container'
  , 'python'
  , 'qvm'
  , 'random'
  , 'range'
  , 'ratio'
  , 'rational'
  , 'regex'
  , 'safe_numerics'
  , 'scope_exit'
  , 'serialization'
  , 'signals2'
  , 'smart_ptr'
  , 'sort'
  , 'spirit'
  , 'stacktrace'
  , 'statechart'
  , 'static_assert'
  , 'static_string'
  , 'stl_interfaces'
  , 'system'
  , 'test'
  , 'thread'
  , 'throw_exception'
  , 'timer'
  , 'tokenizer'
  , 'tti'
  , 'tuple'
  , 'type_erasure'
  , 'type_index'
  , 'type_traits'
  , 'typeof'
  , 'units'
  , 'unordered'
  , 'utility'
  , 'uuid'
  , 'variant'
  , 'variant2'
  , 'vmd'
  , 'wave'
  , 'winapi'
  , 'xpressive'
  , 'yap'
  ]
"""
