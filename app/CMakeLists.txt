# Build the app which supports ALSA, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(as4_finddot ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(as4_finddot LINK_PRIVATE hal)

# ---------------- NEW! ----------------
# ALSA support
find_package(ALSA REQUIRED)
target_link_libraries(as4_finddot LINK_PRIVATE asound)

# PTHREAD support
find_package(Threads REQUIRED)
target_link_libraries(as4_finddot LINK_PRIVATE Threads::Threads)


# Copy executable to final location (change `wave_player_cmake` to project name as needed)
add_custom_command(TARGET as4_finddot POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:as4_finddot>"
     "~/cmpt433/public/myApps/as4-finddot" 
  COMMENT "Copying ARM executable to public NFS directory")


# ---------------- NEW! ----------------
# Copy the WAV folder to NFS
# add_custom_command(TARGET wave_player_cmake POST_BUILD 
#   COMMAND "${CMAKE_COMMAND}" -E copy_directory
#      "${CMAKE_SOURCE_DIR}/wave-files"
#      "~/cmpt433/public/myApps/wave-files" 
#   COMMENT "Copying WAVE files to public NFS directory")
# ---------------- NEW! ----------------
# Copy the PRU folder to NFS. Code adapted from wave_files copy-over from as3
# add_custom_command(TARGET as4_finddot POST_BUILD 
#   COMMAND "${CMAKE_COMMAND}" -E copy_directory
#      "${CMAKE_SOURCE_DIR}/hal/pru"
#      "~/cmpt433/public/myApps/as4-pru" 
#   COMMENT "Copying PRU files to public NFS directory")