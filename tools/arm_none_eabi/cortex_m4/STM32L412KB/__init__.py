from tools.arm_none_eabi.cortex_m4 import generate as generate_parent
from tools.arm_none_eabi.cortex_m4 import phrase_args


c_defines = { 'USE_HAL_DRIVER':None, 'STM32L412xx':None }


def generate(env, **kwargs):
	generate_parent(env, **kwargs)
	env.Append(CCFLAGS=phrase_args('-D', **c_defines))
	env.Append(CPPPATH=['#Drivers/CMSIS/Device/ST/STM32L4xx/Include'])
	env.Append(LINKFLAGS=['-TSTM32L412KBUx_FLASH.ld'])


def exists(env):
	return true

