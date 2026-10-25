# qm-dsp (Queen Mary DSP, GPL-2.0-or-later) subset used for BPM, beatgrid and
# key. Sources come from FetchContent (cmake/Dependencies.cmake). Only the
# files reachable from TempoTrackV2, DetectionFunction and GetKeyMode are
# built; the rest of the library (clapack, cblas, ...) is left out.
set(TURBITO_QMDSP_SOURCES
    base/KaiserWindow.cpp
    base/Pitch.cpp
    base/SincWindow.cpp
    dsp/chromagram/Chromagram.cpp
    dsp/chromagram/ConstantQ.cpp
    dsp/keydetection/GetKeyMode.cpp
    dsp/onsets/DetectionFunction.cpp
    dsp/phasevocoder/PhaseVocoder.cpp
    dsp/rateconversion/Decimator.cpp
    dsp/tempotracking/TempoTrackV2.cpp
    dsp/transforms/FFT.cpp
    maths/MathUtilities.cpp
    ext/kissfft/kiss_fft.c
    ext/kissfft/tools/kiss_fftr.c)

list(TRANSFORM TURBITO_QMDSP_SOURCES PREPEND ${qmdsp_SOURCE_DIR}/)

add_library(turbito_qmdsp STATIC ${TURBITO_QMDSP_SOURCES})

# kiss_fft_scalar must match in every translation unit that sees kiss_fft.h,
# so it is PUBLIC.
target_include_directories(turbito_qmdsp SYSTEM PUBLIC
    ${qmdsp_SOURCE_DIR}
    ${qmdsp_SOURCE_DIR}/ext/kissfft
    ${qmdsp_SOURCE_DIR}/ext/kissfft/tools)

# MSVC hides M_PI unless _USE_MATH_DEFINES is set, and some qm-dsp headers
# use it, so it is PUBLIC for the consumers too.
target_compile_definitions(turbito_qmdsp
    PUBLIC kiss_fft_scalar=double
    PUBLIC $<$<PLATFORM_ID:Windows>:_USE_MATH_DEFINES>
    PRIVATE $<$<PLATFORM_ID:Windows>:NOMINMAX _CRT_SECURE_NO_WARNINGS>)

# Third-party code: keep its warnings out of our build.
if(MSVC)
    target_compile_options(turbito_qmdsp PRIVATE /W0)
else()
    target_compile_options(turbito_qmdsp PRIVATE -w)
endif()
