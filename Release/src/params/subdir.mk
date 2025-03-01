################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/params/Formation.cpp \
../src/params/FormationTactics.cpp \
../src/params/ParamEngine.cpp \
../src/params/PlayerParam.cpp \
../src/params/ServerParam.cpp 

OBJS += \
./src/params/Formation.o \
./src/params/FormationTactics.o \
./src/params/ParamEngine.o \
./src/params/PlayerParam.o \
./src/params/ServerParam.o 

CPP_DEPS += \
./src/params/Formation.d \
./src/params/FormationTactics.d \
./src/params/ParamEngine.d \
./src/params/PlayerParam.d \
./src/params/ServerParam.d 


# Each subdirectory must supply rules for building sources it contributes
src/params/%.o: ../src/params/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '