# ################################################################################
# # LightMeshRT: Radio Driver subdir.mk
# ################################################################################

# # Step 1: Source file discovery
# RADIO_SRCS = $(wildcard ../lib/radio/*.c)
# RADIO_OBJS = $(RADIO_SRCS:.c=.o)
# RADIO_DEPS = $(RADIO_SRCS:.c=.d)

# # Step 2: Register objects and dependencies
# OBJS += $(subst ../, ./mtkernel_3/, $(RADIO_OBJS))
# C_DEPS += $(subst ../, ./mtkernel_3/, $(RADIO_DEPS))

# # Step 3: Compilation rule (same format as app_sample2)
# mtkernel_3/lib/radio/%.o: ../lib/radio/%.c
# 	@echo 'Building file: $<'
# 	$(GCC) $(CFLAGS) -D$(TARGET) $(INCPATH) -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
# 	@echo 'Finished building: $<'
# 	@echo ' '


################################################################################
# Final working version - mtkernel_3/lib/radio/subdir.mk
################################################################################

RADIO_OBJ := mtkernel_3/lib/radio/radio_driver.o
RADIO_SRC := mtkernel_3/lib/radio/radio_driver.c
RADIO_DEP := mtkernel_3/lib/radio/radio_driver.d

OBJS += $(RADIO_OBJ)
C_DEPS += $(RADIO_DEP)

$(RADIO_OBJ): $(RADIO_SRC)
	@echo 'Compiling: $<'
	$(GCC) $(CFLAGS) -D$(TARGET) $(INCPATH) -MF"$(RADIO_DEP)" -MT"$@" -c -o "$@" "$<"
	@echo 'Done: $<'

# 	@echo 'Finished: $<'
