
CFLAGS += -Wall -Wformat=2 -Wswitch-default -Wswitch-enum -Wcast-align
CFLAGS += -Wstrict-overflow=5
CFLAGS += -Winline -Wundef -Wcast-qual -Wshadow
CFLAGS += -Wlogical-op -Wstrict-aliasing=2 -Wredundant-decls
CFLAGS += -Wmissing-include-dirs -Wunused -Wuninitialized
CFLAGS += -Wmissing-declarations -Wunreachable-code
# Example ((float)x == 0.0)
CFLAGS += -Wfloat-equal
# Wvla - warn variable length array
CFLAGS += -Wvla
# Warn if compiler made an assumption about some loop condition
CFLAGS += -Wunsafe-loop-optimizations
# Cortex FPU can be slow on doubles
CFLAGS += -Wdouble-promotion
# warn on write to string in program memory (strings that should be const)
CFLAGS += -Wwrite-strings
# catch bad loop condition etc
CFLAGS += -Wsign-compare
# sizeof(void) is only defined in gcc
CFLAGS += -Wpointer-arith
# if detectable at runtime
CFLAGS += -Wno-div-by-zero

CFLAGS += -Wbad-function-cast -Wstrict-prototypes -Wnested-externs
CFLAGS += -Wold-style-definition -Wmissing-parameter-type
#CFLAGS += -Wsign-conversion

####### PERMANENTLY DISABLE  ###########
CFLAGS += -Wno-sign-compare
CFLAGS += -Wno-switch-enum

####### TEMPORARY DISABLE ###########
CFLAGS += -Wno-missing-declarations
#CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-missing-declarations

######## Usefule for debug otherwise spamy (temporary enable) ########
#CFLAGS += -Wsign-conversion
# Warn if padding in structs. (spamy0)
#CFLAGS += -Wpadded
#CFLAGS += -Wtraditional-conversion
# example warn on ((uint32_t)x < 0) ...
#CFLAGS += -Wtype-limits

########  GCC > 4.7 ########
# heartbleed. gcc >4.9?
# CFLAGS += -Wmisleading-indentation
# CFLAGS += -Wfloat-conversion
# example: (-1) << 1 gcc >4.9?
#CFLAGS += -Wshift-negative-value
# example: 1 << (int32_t). gcc >4.9?
#CFLAGS += -Wshift-count-negative
# Compiler ignored some attribute
#CFLAGS += -Wignored-attributes
# set to some sane value?
#-Wstack-usage=len
