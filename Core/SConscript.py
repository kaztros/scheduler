
Import('env')
env.Append(CPPPATH=['#/Drivers/STM32L4xx_HAL_Driver/Inc'])
mainObjs = env.Object(source='Src/startup_stm32l412xx.S')
mainObjs += env.StaticObject(source = Glob('Src/*.c') + Glob('Src/*.cpp') + Glob('Src/CTM/*.cpp'))
mainLib = env.StaticLibrary(target = 'PWMDivider', source = mainObjs)
Return('mainLib')
