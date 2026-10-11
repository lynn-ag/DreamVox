cmake_minimum_required(VERSION 3.22)
project(DreamVox VERSION 0.2.0 LANGUAGES C CXX)

if(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
  add_subdirectory("${JUCE_DIR}" JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.13
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)
endif()

juce_add_plugin(DreamVox
  COMPANY_NAME "DreamVox Audio"
  BUNDLE_ID "audio.dreamvox.vocalrack"
  PLUGIN_MANUFACTURER_CODE DmVx
  PLUGIN_CODE DrVx
  PRODUCT_NAME "DreamVox"
  DESCRIPTION "Dreamy vocal mixing rack"
  FORMATS VST3
  VST3_CATEGORIES Fx)

target_sources(DreamVox PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)
target_compile_features(DreamVox PUBLIC cxx_std_17)
target_compile_definitions(DreamVox PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(DreamVox PRIVATE
  juce::juce_audio_utils
  juce::juce_audio_processors
  juce::juce_gui_extra
  PUBLIC juce::juce_recommended_config_flags
         juce::juce_recommended_lto_flags
         juce::juce_recommended_warning_flags)
