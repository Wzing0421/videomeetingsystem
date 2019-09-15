
Something may need to change before compile:
in each module's secure function
Some cpuid may be 19 bits or 20 bits
if 19bits cfg1[19] = '\0'
if 20bits cfg1[20] = '\0'

make KEY=\\\"yourCpuIDCOmbine\\\" to compile
