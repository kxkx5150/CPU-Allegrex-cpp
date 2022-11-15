cmake_minimum_required(VERSION 3.10)
project(psp)
enable_language(ASM)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_C_STANDARD 11) 
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -ffast-math")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -D_NDEBUG -s")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ffast-math -g -D_NDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -ffast-math -D_NDEBUG")

macro(setup_target_project TargetName ProjectDir)
	get_property(TargetSources TARGET "${TargetName}" PROPERTY SOURCES)
	foreach(Source ${TargetSources})
		string(REGEX REPLACE "${ProjectDir}" "" RelativePath "${Source}")
		string(REGEX REPLACE "[\\\\/][^\\\\/]*$" "" RelativePath "${RelativePath}")
		string(REGEX REPLACE "^[\\\\/]" "" RelativePath "${RelativePath}")
		string(REGEX REPLACE "/" "\\\\\\\\" RelativePath "${RelativePath}")
		source_group("${RelativePath}" FILES ${Source})
	endforeach()
endmacro()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/exe)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)

add_definitions(-DPPSSPP)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DGLEW_STATIC)
add_definitions(-DSDL)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D_ARCH_64=1)
add_definitions(-D_M_X64)

set(X86 ON)
set(LINUX ON)
set(_ARCH_64 1)
set(CoreExtra)
set(CoreExtraLibs)
set(CoreLibName Core)
set(CoreLinkType STATIC)
set(CommonExtra)
set(nativeExtra)
set(nativeExtraLibs)
set(RT_LIB rt)

add_subdirectory(src/ext/snappy)
# add_subdirectory(src/ext/udis86)

include_directories(src/native)
include_directories(src/)

include(FindOpenGL REQUIRED)
include(FindThreads)

set(CommonExtra ${CommonExtra}
	src/Common/ABI.cpp
	src/Common/ABI.h
	src/Common/CPUDetect.cpp
	src/Common/CPUDetect.h
	src/Common/Thunk.cpp
	src/Common/Thunk.h
	# src/Common/x64Analyzer.cpp
	# src/Common/x64Analyzer.h
	src/Common/x64Emitter.cpp
	src/Common/x64Emitter.h
)

add_library(Common STATIC
	${CommonExtra}
	src/Common/ChunkFile.cpp
	src/Common/ChunkFile.h
	src/Common/ConsoleListener.cpp
	src/Common/ConsoleListener.h
	src/Common/Crypto/md5.cpp
	src/Common/Crypto/md5.h
	src/Common/Crypto/sha1.cpp
	src/Common/Crypto/sha1.h
	src/Common/Crypto/sha256.cpp
	src/Common/Crypto/sha256.h
	src/Common/FileUtil.cpp
	src/Common/FileUtil.h
	src/Common/KeyMap.cpp
	src/Common/KeyMap.h
	src/Common/LogManager.cpp
	src/Common/LogManager.h
	src/Common/MemArena.cpp
	src/Common/MemArena.h
	src/Common/MemoryUtil.cpp
	src/Common/MemoryUtil.h
	src/Common/Misc.cpp
	src/Common/MsgHandler.cpp
	src/Common/MsgHandler.h
	src/Common/StringUtils.cpp
	src/Common/StringUtils.h
	src/Common/ThreadPools.cpp
	src/Common/ThreadPools.h
	src/Common/Timer.cpp
	src/Common/Timer.h)
	
include_directories(src/Common)
setup_target_project(Common Common)

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})




# opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})







# add_library(vjson STATIC
# 	src/native/ext/vjson/json.cpp
# 	src/native/ext/vjson/json.h
# 	src/native/ext/vjson/block_allocator.cpp
# 	src/native/ext/vjson/block_allocator.h
# )
add_library(rg_etc1 STATIC
	src/native/ext/rg_etc1/rg_etc1.cpp
	src/native/ext/rg_etc1/rg_etc1.h)
include_directories(src/native/ext/rg_etc1)
# add_library(stb_vorbis STATIC
# 	src/native/ext/stb_vorbis/stb_vorbis.c
# 	src/native/ext/stb_vorbis/stb_vorbis.h)
# include_directories(src/native/ext/stb_vorbis)

add_library(cityhash STATIC
	src/native/ext/cityhash/city.cpp
	src/native/ext/cityhash/city.h
	src/native/ext/cityhash/citycrc.h
)
include_directories(src/ext/cityhash)

set(nativeExtra ${nativeExtra} src/native/math/fast/fast_matrix_sse.c)

set(nativeExtraLibs ${nativeExtraLibs} SDL2_image SDL2_ttf SDL2 SDL2main)

add_library(native STATIC
	${nativeExtra}
	src/native/base/backtrace.cpp
	src/native/base/backtrace.h
	# src/native/audio/mixer.cpp
	# src/native/audio/mixer.h
	# src/native/audio/wav_read.cpp
	# src/native/audio/wav_read.h
	src/native/base/basictypes.h
	src/native/base/buffer.cpp
	src/native/base/buffer.h
	src/native/base/color.h
	src/native/base/colorutil.cpp
	src/native/base/colorutil.h
	src/native/base/display.cpp
	src/native/base/display.h
	src/native/base/error_context.cpp
	src/native/base/error_context.h
	src/native/base/fastlist.h
	src/native/base/fastlist_test.cpp
	src/native/base/functional.h
	src/native/base/linked_ptr.h
	src/native/base/logging.h
	src/native/base/mutex.h
	src/native/base/scoped_ptr.h
	src/native/base/stats.h
	src/native/base/stringutil.cpp
	src/native/base/stringutil.h
	src/native/base/timeutil.cpp
	src/native/base/timeutil.h
	src/native/data/compression.cpp
	src/native/data/compression.h
	# src/native/ext/vjson/json.cpp
	# src/native/ext/vjson/json.h
	# src/native/ext/vjson/block_allocator.cpp
	# src/native/ext/vjson/block_allocator.h
	src/native/file/chunk_file.cpp
	src/native/file/chunk_file.h
	src/native/file/dialog.cpp
	src/native/file/dialog.h
	src/native/file/easy_file.cpp
	src/native/file/easy_file.h
	src/native/file/fd_util.cpp
	src/native/file/fd_util.h
	src/native/file/file_util.cpp
	src/native/file/file_util.h
	src/native/file/ini_file.cpp
	src/native/file/ini_file.h
	src/native/file/path.cpp
	src/native/file/path.h
	src/native/file/vfs.h
	src/native/file/zip_read.cpp
	src/native/file/zip_read.h
	src/native/gfx/gl_common.h
	src/native/gfx/gl_debug_log.cpp
	src/native/gfx/gl_debug_log.h
	src/native/gfx/gl_lost_manager.cpp
	src/native/gfx/gl_lost_manager.h
	src/native/gfx/texture.cpp
	src/native/gfx/texture.h
	src/native/gfx/texture_atlas.cpp
	src/native/gfx/texture_atlas.h
	src/native/gfx/texture_gen.cpp
	src/native/gfx/texture_gen.h
	src/native/gfx_es2/draw_buffer.cpp
	src/native/gfx_es2/draw_buffer.h
	src/native/gfx_es2/draw_text.cpp
	src/native/gfx_es2/draw_text.h
	src/native/gfx_es2/fbo.cpp
	src/native/gfx_es2/fbo.h
	src/native/gfx_es2/gl_state.cpp
	src/native/gfx_es2/gl_state.h
	src/native/gfx_es2/gpu_features.cpp
	src/native/gfx_es2/gpu_features.h
	src/native/gfx_es2/glsl_program.cpp
	src/native/gfx_es2/glsl_program.h
	src/native/gfx_es2/vertex_format.cpp
	src/native/gfx_es2/vertex_format.h
	src/native/gfx_es2/gl3stub.c
	src/native/gfx_es2/gl3stub.h
	src/native/i18n/i18n.cpp
	src/native/i18n/i18n.h
	src/native/image/png_load.cpp
	src/native/image/png_load.h
	src/native/image/zim_load.cpp
	src/native/image/zim_load.h
	src/native/image/zim_save.cpp
	src/native/image/zim_save.h
	src/native/input/gesture_detector.cpp
	src/native/input/gesture_detector.h
	src/native/input/keycodes.h
	src/native/input/input_state.h
	src/native/input/input_state.cpp
	src/native/math/fast/fast_math.c
	src/native/math/fast/fast_matrix.c
	src/native/math/curves.cpp
	src/native/math/curves.h
	src/native/math/expression_parser.cpp
	src/native/math/expression_parser.h
	src/native/math/lin/aabb.cpp
	src/native/math/lin/aabb.h
	src/native/math/lin/matrix4x4.cpp
	src/native/math/lin/matrix4x4.h
	src/native/math/lin/plane.cpp
	src/native/math/lin/plane.h
	src/native/math/lin/quat.cpp
	src/native/math/lin/quat.h
	src/native/math/lin/ray.h
	src/native/math/lin/vec3.cpp
	src/native/math/lin/vec3.h
	src/native/math/math_util.cpp
	src/native/math/math_util.h
	src/native/net/http_client.cpp
	src/native/net/http_client.h
	src/native/net/resolve.cpp
	src/native/net/resolve.h
	src/native/net/url.cpp
	src/native/net/url.h
	# src/native/profiler/profiler.cpp
	# src/native/profiler/profiler.h
	src/native/thin3d/thin3d.cpp
	src/native/thin3d/thin3d.h
	src/native/thin3d/thin3d_gl.cpp
	src/native/thread/prioritizedworkqueue.cpp
	src/native/thread/prioritizedworkqueue.h
	src/native/thread/threadutil.cpp
	src/native/thread/threadutil.h
	src/native/thread/threadpool.cpp
	src/native/thread/threadpool.h
	src/native/ui/screen.cpp
	src/native/ui/screen.h
	src/native/ui/ui.cpp
	src/native/ui/ui.h
	src/native/ui/ui_context.cpp
	src/native/ui/ui_context.h
	src/native/ui/ui_screen.cpp
	src/native/ui/ui_screen.h
	src/native/ui/view.cpp
	src/native/ui/view.h
	src/native/ui/viewgroup.cpp
	src/native/ui/viewgroup.h
	# src/native/ui/virtual_input.cpp
	# src/native/ui/virtual_input.h
	src/native/ui/EmuScreen.cpp
	src/native/ui/EmuScreen.h

	src/native/util/bits/bits.cpp
	src/native/util/bits/bits.h
	src/native/util/bits/hamming.h
	src/native/util/bits/varint.cpp
	src/native/util/bits/varint.h
	src/native/util/hash/hash.cpp
	src/native/util/hash/hash.h
	src/native/util/random/perlin.cpp
	src/native/util/random/perlin.h
	src/native/util/random/rng.h
	src/native/util/text/utf8.h
	src/native/util/text/utf8.cpp
	src/native/util/text/parsers.h
	src/native/util/text/parsers.cpp
	# src/native/util/const_map.h
	src/native/ext/jpge/jpgd.cpp
	src/native/ext/jpge/jpgd.h
	src/native/ext/jpge/jpge.cpp
	src/native/ext/jpge/jpge.h)
include_directories(native)


target_link_libraries(native ${ZLIB_LIBRARY} rg_etc1 snappy ${RT_LIB} ${GLEW_LIBRARIES})

setup_target_project(native native)

add_library(kirk STATIC
	src/ext/libkirk/AES.c
	src/ext/libkirk/AES.h
	src/ext/libkirk/amctrl.c
	src/ext/libkirk/amctrl.h
	src/ext/libkirk/SHA1.c
	src/ext/libkirk/SHA1.h
	src/ext/libkirk/bn.c
	src/ext/libkirk/ec.c
	src/ext/libkirk/kirk_engine.c
	src/ext/libkirk/kirk_engine.h)
include_directories(src/ext/libkirk)


add_library(xbrz STATIC
	src/ext/xbrz/xbrz.cpp
	src/ext/xbrz/xbrz.h
)
include_directories(src/ext/xbrz)

add_library(xxhash STATIC
	src/ext/xxhash.c
	src/ext/xxhash.h
)
include_directories(src/ext/xxhash)

set(CoreExtra ${CoreExtra}
	src/Core/MIPS/x86/Asm.cpp
	src/Core/MIPS/x86/Asm.h
	src/Core/MIPS/x86/CompALU.cpp
	src/Core/MIPS/x86/CompBranch.cpp
	src/Core/MIPS/x86/CompFPU.cpp
	src/Core/MIPS/x86/CompLoadStore.cpp
	src/Core/MIPS/x86/CompVFPU.cpp
	src/Core/MIPS/x86/CompReplace.cpp
	src/Core/MIPS/x86/Jit.cpp
	src/Core/MIPS/x86/Jit.h
	src/Core/MIPS/x86/JitSafeMem.cpp
	src/Core/MIPS/x86/JitSafeMem.h
	src/Core/MIPS/x86/RegCache.cpp
	src/Core/MIPS/x86/RegCache.h
	src/Core/MIPS/x86/RegCacheFPU.cpp
	src/Core/MIPS/x86/RegCacheFPU.h
	src/GPU/Common/VertexDecoderX86.cpp
	)

add_library(${CoreLibName} ${CoreLinkType}
	${CoreExtra}
	src/Core/Config.cpp
	src/Core/Config.h
	src/Core/Core.cpp
	src/Core/Core.h
	src/Core/CoreParameter.h
	src/Core/CoreTiming.cpp
	src/Core/CoreTiming.h
	# src/Core/CwCheat.cpp
	# src/Core/CwCheat.h
	src/Core/HDRemaster.cpp
	src/Core/HDRemaster.h
	src/Core/ThreadEventQueue.h
	src/Core/Debugger/Breakpoints.cpp
	src/Core/Debugger/Breakpoints.h
	src/Core/Debugger/DebugInterface.h
	src/Core/Debugger/SymbolMap.cpp
	src/Core/Debugger/SymbolMap.h
	src/Core/Debugger/DisassemblyManager.cpp
	src/Core/Debugger/DisassemblyManager.h
	src/Core/Dialog/PSPDialog.cpp
	src/Core/Dialog/PSPDialog.h
	src/Core/Dialog/PSPGamedataInstallDialog.cpp
	src/Core/Dialog/PSPGamedataInstallDialog.h
	src/Core/Dialog/PSPMsgDialog.cpp
	src/Core/Dialog/PSPMsgDialog.h
	src/Core/Dialog/PSPNetconfDialog.cpp
	src/Core/Dialog/PSPNetconfDialog.h
	src/Core/Dialog/PSPOskDialog.cpp
	src/Core/Dialog/PSPOskDialog.h
	src/Core/Dialog/PSPPlaceholderDialog.cpp
	src/Core/Dialog/PSPPlaceholderDialog.h
	src/Core/Dialog/PSPSaveDialog.cpp
	src/Core/Dialog/PSPSaveDialog.h
	src/Core/Dialog/PSPScreenshotDialog.cpp
	src/Core/Dialog/PSPScreenshotDialog.h
	src/Core/Dialog/SavedataParam.cpp
	src/Core/Dialog/SavedataParam.h
	src/Core/ELF/ElfReader.cpp
	src/Core/ELF/ElfReader.h
	src/Core/ELF/ElfTypes.h
	src/Core/ELF/PBPReader.cpp
	src/Core/ELF/PBPReader.h
	src/Core/ELF/PrxDecrypter.cpp
	src/Core/ELF/PrxDecrypter.h
	src/Core/ELF/ParamSFO.cpp
	src/Core/ELF/ParamSFO.h
	src/Core/FileSystems/tlzrc.cpp
	src/Core/FileSystems/BlockDevices.cpp
	src/Core/FileSystems/BlockDevices.h
	src/Core/FileSystems/DirectoryFileSystem.cpp
	src/Core/FileSystems/DirectoryFileSystem.h
	src/Core/FileSystems/FileSystem.h
	src/Core/FileSystems/FileSystem.cpp
	src/Core/FileSystems/ISOFileSystem.cpp
	src/Core/FileSystems/ISOFileSystem.h
	src/Core/FileSystems/MetaFileSystem.cpp
	src/Core/FileSystems/MetaFileSystem.h
	src/Core/FileSystems/VirtualDiscFileSystem.cpp
	src/Core/FileSystems/VirtualDiscFileSystem.h
	src/Core/Font/PGF.cpp
	src/Core/Font/PGF.h
	src/Core/HLE/FunctionWrappers.h
	src/Core/HLE/HLE.cpp
	src/Core/HLE/HLE.h
	src/Core/HLE/ReplaceTables.cpp
	src/Core/HLE/ReplaceTables.h
	src/Core/HLE/HLEHelperThread.cpp
	src/Core/HLE/HLEHelperThread.h
	src/Core/HLE/HLETables.cpp
	src/Core/HLE/HLETables.h
	src/Core/HLE/KernelWaitHelpers.h
	src/Core/HLE/__sceAudio.cpp
	src/Core/HLE/__sceAudio.h
	src/Core/HLE/sceAdler.cpp
	src/Core/HLE/sceAdler.h
	src/Core/HLE/sceAtrac.cpp
	src/Core/HLE/sceAtrac.h
	src/Core/HLE/sceAudio.cpp
	src/Core/HLE/sceAudiocodec.cpp
	src/Core/HLE/sceAudiocodec.h
	src/Core/HLE/sceAudio.h
	src/Core/HLE/sceCcc.h
	src/Core/HLE/sceCcc.cpp
	src/Core/HLE/sceChnnlsv.cpp
	src/Core/HLE/sceChnnlsv.h
	src/Core/HLE/sceCtrl.cpp
	src/Core/HLE/sceCtrl.h
	src/Core/HLE/sceDeflt.cpp
	src/Core/HLE/sceDeflt.h
	src/Core/HLE/sceDisplay.cpp
	src/Core/HLE/sceDisplay.h
	src/Core/HLE/sceDmac.cpp
	src/Core/HLE/sceDmac.h
	src/Core/HLE/sceGameUpdate.cpp
	src/Core/HLE/sceGameUpdate.h
	src/Core/HLE/sceGe.cpp
	src/Core/HLE/sceGe.h
	src/Core/HLE/sceFont.cpp
	src/Core/HLE/sceFont.h
	src/Core/HLE/sceHeap.cpp
	src/Core/HLE/sceHeap.h
	src/Core/HLE/sceHprm.cpp
	src/Core/HLE/sceHprm.h
	src/Core/HLE/sceHttp.cpp
	src/Core/HLE/sceHttp.h
	src/Core/HLE/sceImpose.cpp
	src/Core/HLE/sceImpose.h
	src/Core/HLE/sceIo.cpp
	src/Core/HLE/sceIo.h
	src/Core/HLE/sceJpeg.cpp
	src/Core/HLE/sceJpeg.h
	src/Core/HLE/sceKernel.cpp
	src/Core/HLE/sceKernel.h
	src/Core/HLE/sceKernelAlarm.cpp
	src/Core/HLE/sceKernelAlarm.h
	src/Core/HLE/sceKernelEventFlag.cpp
	src/Core/HLE/sceKernelEventFlag.h
	src/Core/HLE/sceKernelInterrupt.cpp
	src/Core/HLE/sceKernelInterrupt.h
	src/Core/HLE/sceKernelMbx.cpp
	src/Core/HLE/sceKernelMbx.h
	src/Core/HLE/sceKernelMemory.cpp
	src/Core/HLE/sceKernelMemory.h
	src/Core/HLE/sceKernelModule.cpp
	src/Core/HLE/sceKernelModule.h
	src/Core/HLE/sceKernelMsgPipe.cpp
	src/Core/HLE/sceKernelMsgPipe.h
	src/Core/HLE/sceKernelMutex.cpp
	src/Core/HLE/sceKernelMutex.h
	src/Core/HLE/sceKernelSemaphore.cpp
	src/Core/HLE/sceKernelSemaphore.h
	src/Core/HLE/sceKernelThread.cpp
	src/Core/HLE/sceKernelThread.h
	src/Core/HLE/sceKernelTime.cpp
	src/Core/HLE/sceKernelTime.h
	src/Core/HLE/sceKernelVTimer.cpp
	src/Core/HLE/sceKernelVTimer.h
	src/Core/HLE/sceMpeg.cpp
	src/Core/HLE/sceMpeg.h
	src/Core/HLE/sceNet.cpp
	src/Core/HLE/sceNet.h
	src/Core/HLE/sceNetAdhoc.cpp
	src/Core/HLE/sceNetAdhoc.h
	src/Core/HLE/proAdhoc.h
	src/Core/HLE/proAdhoc.cpp
	src/Core/HLE/proAdhocServer.h
	src/Core/HLE/proAdhocServer.cpp
	src/Core/HLE/sceOpenPSID.cpp
	src/Core/HLE/sceOpenPSID.h
	src/Core/HLE/sceP3da.cpp
	src/Core/HLE/sceP3da.h
	# src/Core/HLE/sceMt19937.cpp
	# src/Core/HLE/sceMt19937.h
	src/Core/HLE/sceMd5.cpp
	src/Core/HLE/sceMd5.h
	src/Core/HLE/sceMp4.cpp
	src/Core/HLE/sceMp4.h
	src/Core/HLE/sceMp3.cpp
	src/Core/HLE/sceMp3.h
	src/Core/HLE/sceParseHttp.cpp
	src/Core/HLE/sceParseHttp.h
	src/Core/HLE/sceParseUri.cpp
	src/Core/HLE/sceParseUri.h
	src/Core/HLE/scePower.cpp
	src/Core/HLE/scePower.h
	src/Core/HLE/scePsmf.cpp
	src/Core/HLE/scePsmf.h
	src/Core/HLE/sceRtc.cpp
	src/Core/HLE/sceRtc.h
	src/Core/HLE/sceSas.cpp
	src/Core/HLE/sceSas.h
	# src/Core/HLE/sceSfmt19937.cpp
	# src/Core/HLE/sceSfmt19937.h
	src/Core/HLE/sceSha256.cpp
	src/Core/HLE/sceSha256.h
	src/Core/HLE/sceSsl.cpp
	src/Core/HLE/sceSsl.h
	src/Core/HLE/sceUmd.cpp
	src/Core/HLE/sceUmd.h
	src/Core/HLE/sceUsb.cpp
	src/Core/HLE/sceUsb.h
	src/Core/HLE/sceUsbGps.cpp
	src/Core/HLE/sceUsbGps.h
	src/Core/HLE/sceUtility.cpp
	src/Core/HLE/sceUtility.h
	src/Core/HLE/sceVaudio.cpp
	src/Core/HLE/sceVaudio.h
	src/Core/HLE/scePspNpDrm_user.cpp
	src/Core/HLE/scePspNpDrm_user.h
	src/Core/HLE/sceNp.cpp
	src/Core/HLE/sceNp.h
	src/Core/HLE/scePauth.cpp
	src/Core/HLE/scePauth.h
	src/Core/HW/SimpleAudioDec.cpp
	src/Core/HW/SimpleAudioDec.h
	src/Core/HW/AsyncIOManager.cpp
	src/Core/HW/AsyncIOManager.h
	src/Core/HW/MediaEngine.cpp
	src/Core/HW/MediaEngine.h
	src/Core/HW/MpegDemux.cpp
	src/Core/HW/MpegDemux.h
	src/Core/HW/MemoryStick.cpp
	src/Core/HW/MemoryStick.h
	src/Core/HW/SasAudio.cpp
	src/Core/HW/SasAudio.h
	src/Core/HW/StereoResampler.cpp
	src/Core/HW/StereoResampler.h
	src/Core/Host.cpp
	src/Core/Host.h
	src/Core/Loaders.cpp
	src/Core/Loaders.h
	src/Core/MIPS/JitCommon/JitCommon.cpp
	src/Core/MIPS/JitCommon/JitCommon.h
	src/Core/MIPS/JitCommon/NativeJit.h
	src/Core/MIPS/JitCommon/JitBlockCache.cpp
	src/Core/MIPS/JitCommon/JitBlockCache.h
	src/Core/MIPS/MIPS.cpp
	src/Core/MIPS/MIPS.h
	src/Core/MIPS/MIPSAnalyst.cpp
	src/Core/MIPS/MIPSAnalyst.h
	src/Core/MIPS/MIPSCodeUtils.cpp
	src/Core/MIPS/MIPSCodeUtils.h
	src/Core/MIPS/MIPSDebugInterface.cpp
	src/Core/MIPS/MIPSDebugInterface.h
	src/Core/MIPS/MIPSDis.cpp
	src/Core/MIPS/MIPSDis.h
	src/Core/MIPS/MIPSDisVFPU.cpp
	src/Core/MIPS/MIPSDisVFPU.h
	src/Core/MIPS/MIPSInt.cpp
	src/Core/MIPS/MIPSInt.h
	src/Core/MIPS/MIPSIntVFPU.cpp
	src/Core/MIPS/MIPSIntVFPU.h
	src/Core/MIPS/MIPSStackWalk.cpp
	src/Core/MIPS/MIPSStackWalk.h
	src/Core/MIPS/MIPSTables.cpp
	src/Core/MIPS/MIPSTables.h
	src/Core/MIPS/MIPSVFPUUtils.cpp
	src/Core/MIPS/MIPSVFPUUtils.h
	src/Core/MIPS/MIPSAsm.cpp
	src/Core/MIPS/MIPSAsm.h
	src/Core/MemMap.cpp
	src/Core/MemMap.h
	src/Core/MemMapFunctions.cpp
	src/Core/PSPLoaders.cpp
	src/Core/PSPLoaders.h
	src/Core/Reporting.cpp
	src/Core/Reporting.h
	src/Core/SaveState.cpp
	src/Core/SaveState.h
	src/Core/Screenshot.cpp
	src/Core/Screenshot.h
	src/Core/System.cpp
	src/Core/System.h
	src/Core/Util/AudioFormat.cpp
	src/Core/Util/AudioFormat.h
	# src/Core/Util/GameManager.cpp
	# src/Core/Util/GameManager.h
	src/Core/Util/BlockAllocator.cpp
	src/Core/Util/BlockAllocator.h
	src/Core/Util/PPGeDraw.cpp
	src/Core/Util/PPGeDraw.h
	src/Core/Util/ppge_atlas.cpp
	src/Core/Util/ppge_atlas.h
	${CORE_NEON}
	$<TARGET_OBJECTS:GPU>
	src/Globals.h
	# git-version.cpp
)
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
if (PNG_FOUND)
	message(STATUS "USE_PNG")
	include_directories(${PNG_PNG_INCLUDE_DIR})
endif()

target_link_libraries(${CoreLibName} Common native kirk cityhash xbrz xxhash
	${CoreExtraLibs} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${PNG_LIBRARY})
setup_target_project(${CoreLibName} Core)

add_library(GPU OBJECT
	src/GPU/Common/FramebufferCommon.cpp
	src/GPU/Common/FramebufferCommon.h
	src/GPU/Common/GPUDebugInterface.h
	src/GPU/Common/DrawEngineCommon.cpp
	src/GPU/Common/DrawEngineCommon.h
	src/GPU/Common/SplineCommon.cpp
	src/GPU/Common/SplineCommon.h
	src/GPU/Common/SoftwareTransformCommon.cpp
	src/GPU/Common/SoftwareTransformCommon.h
	src/GPU/Common/VertexDecoderCommon.cpp
	src/GPU/Common/VertexDecoderCommon.h
	src/GPU/Common/TransformCommon.cpp
	src/GPU/Common/TransformCommon.h
	src/GPU/Common/IndexGenerator.cpp
	src/GPU/Common/IndexGenerator.h
	src/GPU/Common/TextureDecoder.cpp
	src/GPU/Common/TextureDecoder.h
	src/GPU/Common/TextureCacheCommon.cpp
	src/GPU/Common/TextureCacheCommon.h
	${GPU_NEON}
	src/GPU/Common/PostShader.cpp
	src/GPU/Common/PostShader.h
	src/GPU/Common/SplineCommon.h
	# src/GPU/Debugger/Breakpoints.cpp
	# src/GPU/Debugger/Breakpoints.h
	# src/GPU/Debugger/Stepping.cpp
	# src/GPU/Debugger/Stepping.h
	src/GPU/GLES/DepalettizeShader.cpp
	src/GPU/GLES/DepalettizeShader.h
	src/GPU/GLES/GLES_GPU.cpp
	src/GPU/GLES/GLES_GPU.h
	src/GPU/GLES/FragmentShaderGenerator.cpp
	src/GPU/GLES/FragmentShaderGenerator.h
	src/GPU/GLES/FragmentTestCache.cpp
	src/GPU/GLES/FragmentTestCache.h
	src/GPU/GLES/Framebuffer.cpp
	src/GPU/GLES/Framebuffer.h
	src/GPU/GLES/ShaderManager.cpp
	src/GPU/GLES/ShaderManager.h
	src/GPU/GLES/Spline.cpp
	src/GPU/GLES/StateMapping.cpp
	src/GPU/GLES/StateMapping.h
	src/GPU/GLES/StencilBuffer.cpp
	src/GPU/GLES/TextureCache.cpp
	src/GPU/GLES/TextureCache.h
	src/GPU/GLES/TextureScaler.cpp
	src/GPU/GLES/TextureScaler.h
	src/GPU/GLES/TransformPipeline.cpp
	src/GPU/GLES/TransformPipeline.h
	src/GPU/GLES/VertexShaderGenerator.cpp
	src/GPU/GLES/VertexShaderGenerator.h
	src/GPU/GPUInterface.h
	src/GPU/GeDisasm.cpp
	src/GPU/GeDisasm.h
	src/GPU/GPUCommon.cpp
	src/GPU/GPUCommon.h
	src/GPU/GPUState.cpp
	src/GPU/GPUState.h
	src/GPU/Math3D.cpp
	src/GPU/Math3D.h
	# src/GPU/Null/NullGpu.cpp
	# src/GPU/Null/NullGpu.h
	src/GPU/ge_constants.h)
setup_target_project(GPU GPU)

set(LinkCommon ${CoreLibName} ${CMAKE_THREAD_LIBS_INIT} ${nativeExtraLibs})

set(USE_FFMPEG ON)
if(USE_FFMPEG)
	include_directories(ffmpeg/linux/x86_64/include)
	link_directories(ffmpeg/linux/x86_64/lib)
	set(FFMPEG_LIBRARIES libavformat.a libavcodec.a libavutil.a libswresample.a libswscale.a)

	find_library(ICONV_LIBRARY NAMES iconv)
	if (ICONV_LIBRARY)
		set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${ICONV_LIBRARY})
	endif()
	set(LinkCommon ${LinkCommon} ${FFMPEG_LIBRARIES})
	add_definitions(-DUSE_FFMPEG)

	target_link_libraries(Common ${FFMPEG_LIBRARIES})
	message(STATUS "USE_FFMPEG")
endif(USE_FFMPEG AND NOT DEFINED FFMPEG_BUILDDIR)


set(NativeAppSource
	src/main.cpp
)



add_executable(psp ${NativeAppSource})
target_link_libraries(psp ${LinkCommon} ${PNG_LIBRARY} SDL2_image SDL2_ttf SDL2 SDL2main)

