Import('env')
#obj = env.StaticLibrary(target = 'stm32l4xx_hal', source = Glob("*.c"))
env.Append(CPPPATH=['Inc'])
objs = env.StaticObject(Glob('Src/*.c'))
lib = env.StaticLibrary(target = 'STM32L4xx_HAL_Driver', source=objs)
Return('lib')
