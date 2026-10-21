cmake_minimum_required(VERSION 3.22)
project(EMBER175_ReferenceLab VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

add_library(ember_dsp STATIC Source/EmberDSP.cpp)
target_include_directories(ember_dsp PUBLIC Source)
target_compile_features(ember_dsp PUBLIC cxx_std_17)

add_executable(ember_dsp_tests Tests/DSPTests.cpp)
target_link_libraries(ember_dsp_tests PRIVATE ember_dsp)

juce_add_plugin(EMBER175
    COMPANY_NAME "Independent Audio Lab"
    BUNDLE_ID "audio.independent.ember175"
    PLUGIN_MANUFACTURER_CODE InAu
    PLUGIN_CODE Em17
    FORMATS VST3
    PRODUCT_NAME "EMBER 175 Reference Lab"
    DESCRIPTION "Independent tube-compressor research implementation"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Dynamics
)
juce_generate_juce_header(EMBER175)
target_sources(EMBER175 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)
target_link_libraries(EMBER175 PRIVATE
    ember_dsp
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
target_compile_definitions(EMBER175 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
