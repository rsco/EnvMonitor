################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
LcdDriver/Crystalfontz128x128_ST7735.obj: ../LcdDriver/Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv6/ccs_base/arm/include" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/LcdDriver" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/GrLib/grlib" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/GrLib/fonts" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/driverlib/MSP432P4xx" --include_path="C:/ti/ccsv6/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="LcdDriver/Crystalfontz128x128_ST7735.d" --obj_directory="LcdDriver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.obj: ../LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv6/ccs_base/arm/include" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/LcdDriver" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/GrLib/grlib" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/GrLib/fonts" --include_path="C:/Users/oliveir/Desktop/EnvMonitor/EnvMonitor/driverlib/MSP432P4xx" --include_path="C:/ti/ccsv6/ccs_base/arm/include/CMSIS" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.d" --obj_directory="LcdDriver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


