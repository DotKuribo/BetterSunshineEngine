cmake_minimum_required(VERSION 3.8)

project(BetterSunshineEngine VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

option(SMS_FIX_CRASHES "Repair supported crashes in Super Mario Sunshine" ON)
option(SMS_FIX_BUGS "Repair supported bugs in Super Mario Sunshine" ON)
option(SMS_FIX_DOWNWARP "Changes OOB behavior so downwarping ceases" ON)
option(SMS_FIX_YOSHI_NOZZLES "Enhances nozzle behavior while riding Yoshi" ON)
option(SMS_INCLUDE_MUSIC "Includes streamed music features" ON)
option(SMS_INCLUDE_EXTENDED_SHINES "Includes support for 999 shines" ON)
option(SMS_INCLUDE_EXTENDED_OBJECTS "Includes support for custom objects" ON)
option(SMS_INCLUDE_EXTENDED_COLLISION "Includes extended collision types" ON)
option(SMS_INCLUDE_EXTENDED_VISIBILITY "Includes extended render distance" ON)
option(SMS_INCLUDE_EXTENDED_MOVESET "Includes extended moves such as Hover Burst, Hover Slide, and Long Jumping" ON)
option(SMS_INCLUDE_MULTI_CHARACTERS "Includes multiple character support" ON)
option(SMS_INCLUDE_GREEN_YOSHI "Includes Green Yoshi and the free egg system" ON)
option(SMS_INCLUDE_UNDERWATER_FRUIT "Includes non-perishing underwater fruit" ON)
option(SMS_INCLUDE_SLOT_B_SUPPORT "Includes Slot B memcard support" ON)
option(SMS_INCLUDE_BMG_EXTENSION "Includes extended BMG opcodes" ON)
option(SMS_INCLUDE_SHADOW_MARIO_HEALTH "Includes the visibility of Shadow Mario's health" ON)
option(SMS_INCLUDE_DYNAMIC_FALL_DAMAGE "Includes dynamic fall damage in SMS" ON)
option(SMS_INCLUDE_EXCEPTION_HANDLER "Includes the exception handler information" ON)

file(GLOB BETTERSMS_SRC
    "src/*.c"
    "src/*.cpp"
    "src/asm/*.s"
    "src/collision/*.c"
    "src/collision/*.cpp"
    "src/cstd/*.c"
    "src/cstd/*.cpp"
    "src/libs/*.cpp"
    "src/libs/dolphin/*.c"
    "src/patches/*.c"
    "src/patches/*.cpp"
    "src/physics/*.c"
    "src/physics/*.cpp"
    "src/shine/*.c"
    "src/shine/*.cpp"
    "src/sunscript/*.c"
    "src/sunscript/*.cpp"
    "src/yoshi/*.c"
    "src/yoshi/*.cpp"
    #"include/*.h"
    #"include/*.hxx"
)


add_executable(BetterSunshineEngine ${BETTERSMS_SRC} src/cstd/ppceabi.c)
add_subdirectory(lib/sms_interface)

link_directories("C:/Windows/System32")

target_link_libraries(BetterSunshineEngine PUBLIC Dolphin JSystem SMS Kamek Kuribo ${PROJECT_SOURCE_DIR}/lib/libKuriboClang/libKuriboClang.a)
target_include_directories(BetterSunshineEngine PRIVATE include lib/sms_interface/include)

target_compile_options(BetterSunshineEngine PUBLIC
    ${SMS_COMPILE_FLAGS}
)

target_link_options(BetterSunshineEngine PUBLIC
    ${SMS_LINK_FLAGS}
)

target_compile_definitions(BetterSunshineEngine PUBLIC KURIBO_NO_TYPES BETTER_SMS_VERSION="v1.0")

if(SMS_FIX_CRASHES)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_CRASHFIXES)
endif()

if(SMS_FIX_BUGS)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()

if(SMS_FIX_DOWNWARP)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_NO_DOWNWARP)
endif()

if(SMS_FIX_YOSHI_NOZZLES)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_YOSHI_SAVE_NOZZLES)
endif()

if(SMS_INCLUDE_MUSIC)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_CUSTOM_MUSIC)
endif()

if(SMS_INCLUDE_EXTENDED_SHINES)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_EXTRA_SHINES)
endif()

if(SMS_INCLUDE_EXTENDED_OBJECTS)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_EXTRA_OBJECTS)
endif()

if(SMS_INCLUDE_EXTENDED_COLLISION)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_EXTRA_COLLISION)
endif()

if(SMS_INCLUDE_EXTENDED_VISIBILITY)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_EXTENDED_RENDER_DISTANCE)
endif()

if(SMS_INCLUDE_EXTENDED_MOVESET)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_LONG_JUMP
                                                           BETTER_SMS_MULTI_JUMP
                                                           BETTER_SMS_HOVER_BURST
                                                           BETTER_SMS_HOVER_SLIDE
                                                           BETTER_SMS_ROCKET_DIVE)
endif()

if(SMS_INCLUDE_MULTI_CHARACTERS)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_CUSTOM_CHARACTERS)
endif()

if(SMS_INCLUDE_GREEN_YOSHI)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()

if(SMS_INCLUDE_UNDERWATER_FRUIT)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()

if(SMS_INCLUDE_SLOT_B_SUPPORT)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()

if(SMS_INCLUDE_BMG_EXTENSION)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()

if(SMS_INCLUDE_SHADOW_MARIO_HEALTH)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()

if(SMS_INCLUDE_DYNAMIC_FALL_DAMAGE)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()

if(SMS_INCLUDE_EXCEPTION_HANDLER)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()


add_custom_target(${PROJECT_NAME}.kxe ALL DEPENDS ${PROJECT_NAME})
add_custom_command(
	TARGET ${PROJECT_NAME}.kxe
	COMMAND ${PROJECT_SOURCE_DIR}/tools/KuriboConverter.exe ARGS ${PROJECT_NAME} ${PROJECT_NAME}.kxe ${PROJECT_SOURCE_DIR}/maps/${SMS_REGION}.map
)