set(INCLUDES 
    "${PORTAUDIO_INCLUDE_DIR}" 
    "${KWD_SOURCE_DIR}/include")

set(INPUTFOLDER "${KWD_SOURCE_DIR}/inputs")

discover_unit_tests("${INCLUDES}" SENSORY "${INPUTFOLDER}")
