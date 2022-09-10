set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR powerpc)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(triple powerpc-unknown-eabi)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

set(CMAKE_SYSROOT "C:/msys64/mingw64")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")

set(CMAKE_CXX_STANDARD_LIBRARIES "")

set(SMS_COMPILE_FLAGS
	-O1 -DKURIBO_NO_TYPES -fno-inline
	-DGEKKO -fno-exceptions -fno-rtti
	-ffast-math -fpermissive -D__powerpc__
	--target=ppc-linux-eabi -mcpu=750
	-fno-function-sections -fno-data-sections
)
set(SMS_LINK_FLAGS
	-nostdlib -fuse-ld=lld -mllvm
	--march=ppc-linux-eabi -mllvm -mcpu=750 -r
	-fno-function-sections -fno-data-sections
)

set(LIBSTDCPP_VERSION "10.2.0")
set(DKP_PATH "C:/devkitPro/devkitPPC/powerpc-eabi/include")

include_directories(SYSTEM
	"C:/devkitPro/devkitPPC/powerpc-eabi/include/c++/10.2.0"
	"C:/devkitPro/devkitPPC/powerpc-eabi/include"
	"C:/devkitPro/devkitPPC/powerpc-eabi/include/c++/10.2.0/powerpc-eabi"
)

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_OBJCOPY C:/devkitPro/devkitPPC/bin/powerpc-eabi-objcopy.exe CACHE PATH "" FORCE)