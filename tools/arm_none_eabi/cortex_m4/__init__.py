from tools.arm_none_eabi import generate as generate_parent
from tools.arm_none_eabi import phrase_args

arm_options = { 'cpu':'cortex-m4'
	, 'fpu':'fpv4-sp-d16'
	, 'float-abi':'hard'
	, 'thumb':None 
	}

def generate(env, **kwargs):
	generate_parent(env, **kwargs)
	processor_flags = phrase_args('-m', **arm_options)
	env.Append(ASFLAGS=processor_flags)
	env.Append(CCFLAGS=processor_flags)
	env.Append(LINKFLAGS=processor_flags)
	env.Append(CPPPATH=['#Drivers/CMSIS/Include'])


def exists(env):
	return true

