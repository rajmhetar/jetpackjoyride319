################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
ticlang/%.o: ../ticlang/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs2000/ccs/tools/compiler/ti-cgt-armllvm_4.0.1.LTS/bin/tiarmclang" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"/Users/joshhlee614/Downloads/MSPM0_ValvanoWare/ECE319K_Lab7H" -I"/Users/joshhlee614/Downloads/MSPM0_ValvanoWare/ECE319K_Lab7H/Debug" -I"/Applications/ti/mspm0_sdk_2_03_00_07/source/third_party/CMSIS/Core/Include" -I"/Applications/ti/mspm0_sdk_2_03_00_07/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"ticlang/$(basename $(<F)).d_raw" -MT"$(@)" -I"/Users/joshhlee614/Downloads/MSPM0_ValvanoWare/ECE319K_Lab7H/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


