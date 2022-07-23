cmake_minimum_required(VERSION 3.8)

project(BetterSunshineEngine)

set(CMAKE_CXX_STANDARD 17)

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

file(GLOB BETTERSUNSHINEENGINE_SRC
    "src/*.c"
    "src/*.cpp"
    "include/*.h"
    "include/*.hxx"
)

add_executable(BetterSunshineEngine ${BETTERSUNSHINEENGINE_SRC})
add_subdirectory(lib/sms_interface)

# include_directories("include")
target_link_libraries(BetterSunshineEngine JSystem)
target_link_libraries(BetterSunshineEngine Dolphin)
target_link_libraries(BetterSunshineEngine SMS)

target_include_directories(BetterSunshineEngine PUBLIC include)

if(SMS_FIX_CRASHES)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_CRASHFIXES)
endif()

if(SMS_FIX_BUGS)
    target_compile_definitions(BetterSunshineEngine PUBLIC BETTER_SMS_BUGFIXES)
endif()
