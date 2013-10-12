################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/c2000_6.1.5

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./main.obj" \
"./functions.obj" \
"./DSP281x_usDelay.obj" \
"./DSP281x_SysCtrl.obj" \
"./DSP281x_PieVect.obj" \
"./DSP281x_PieCtrl.obj" \
"./DSP281x_Gpio.obj" \
"./DSP281x_GlobalVariableDefs.obj" \
"./DSP281x_ECan.obj" \
"./DSP281x_DefaultIsr.obj" \
"./DSP281x_CodeStartBranch.obj" \
-l"libc.a" \
"../DSP281x_Headers_nonBIOS.cmd" \
"../F2812_EzDSP_RAM_lnk.cmd" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: CAN_Slave.out

# Tool invocations
CAN_Slave.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C2000 Linker'
	"C:/ti/ccsv5/tools/compiler/c2000_6.1.5/bin/cl2000" -v28 -ml -mt -g --diag_warning=225 --display_error_number --diag_wrap=off -z --stack_size=0x400 -m"CAN_Slave.map" --warn_sections -i"C:/ti/ccsv5/tools/compiler/c2000_6.1.5/lib" -i"C:/ti/ccsv5/tools/compiler/c2000_6.1.5/include" --reread_libs --display_error_number --diag_wrap=off --xml_link_info="CAN_Slave_linkInfo.xml" --rom_model -o "CAN_Slave.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C2000_EXECUTABLE_OUTPUTS__QUOTED) "CAN_Slave.out"
	-$(RM) "DSP281x_DefaultIsr.pp" "DSP281x_ECan.pp" "DSP281x_GlobalVariableDefs.pp" "DSP281x_Gpio.pp" "DSP281x_PieCtrl.pp" "DSP281x_PieVect.pp" "DSP281x_SysCtrl.pp" "functions.pp" "main.pp" 
	-$(RM) "DSP281x_CodeStartBranch.obj" "DSP281x_DefaultIsr.obj" "DSP281x_ECan.obj" "DSP281x_GlobalVariableDefs.obj" "DSP281x_Gpio.obj" "DSP281x_PieCtrl.obj" "DSP281x_PieVect.obj" "DSP281x_SysCtrl.obj" "DSP281x_usDelay.obj" "functions.obj" "main.obj" 
	-$(RM) "DSP281x_CodeStartBranch.pp" "DSP281x_usDelay.pp" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
