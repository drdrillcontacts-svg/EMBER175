cmake_minimum_required(VERSION 3.22)

project(EMBER175 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version" FORCE)
endif()

option(EMBER_BUILD_PLUGIN "Build the plug-in targets (needs JUCE + platform SDKs)" ON)

# ---------------------------------------------------------------------------
# JUCE
# ---------------------------------------------------------------------------
if (EMBER_BUILD_PLUGIN)
    include(FetchContent)

    set(JUCE_VERSION "8.0.6")
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG ${JUCE_VERSION}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------------------
# DSP core (no JUCE dependency, so the tests build and run anywhere)
# ---------------------------------------------------------------------------
add_library(ember_dsp STATIC
    Source/EmberDSP.cpp
    Source/EmberDSP.h
    Source/EmberModel.h
)
target_include_directories(ember_dsp PUBLIC Source)
target_compile_features(ember_dsp PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
add_executable(ember_dsp_tests Tests/DSPTests.cpp)
target_link_libraries(ember_dsp_tests PRIVATE ember_dsp)
target_compile_options(ember_dsp_tests PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O2>)

enable_testing()
add_test(NAME dsp COMMAND ember_dsp_tests)

# ---------------------------------------------------------------------------
# Plug-in
# ---------------------------------------------------------------------------
if (EMBER_BUILD_PLUGIN)
set(EMBER_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND EMBER_FORMATS AU)
endif()

juce_add_plugin(EMBER175
    COMPANY_NAME "Ember Audio Lab"
    BUNDLE_ID "audio.ember.lab.ember175"
    PLUGIN_MANUFACTURER_CODE Embr
    PLUGIN_CODE E175
    FORMATS ${EMBER_FORMATS}
    PRODUCT_NAME "EMBER 175-B"
    DESCRIPTION "Tube variable-mu limiting amplifier, modelled after the UA 175-B"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Dynamics
    AU_MAIN_TYPE kAudioUnitType_Effect
    MICROPHONE_PERMISSION_ENABLED FALSE
)

juce_generate_juce_header(EMBER175)

target_sources(EMBER175 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/UI/PanelRenderer.cpp
    Source/UI/PanelRenderer.h
    Source/UI/PanelLayout.cpp
    Source/UI/PanelLayout.h
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
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

# ---------------------------------------------------------------------------
# Optional: render the front panel to a PNG without a display, so the design
# can be reviewed from CI artefacts.
# ---------------------------------------------------------------------------
option(EMBER_BUILD_UI_PREVIEW "Build the headless front panel renderer" OFF)

if (EMBER_BUILD_UI_PREVIEW)
    juce_add_console_app(ember_ui_preview PRODUCT_NAME "ember_ui_preview")
    target_sources(ember_ui_preview PRIVATE
        Tools/UIPreview.cpp
        Source/UI/PanelRenderer.cpp
        Source/UI/PanelLayout.cpp
    )
    target_include_directories(ember_ui_preview PRIVATE Source)
    target_link_libraries(ember_ui_preview PRIVATE
        juce::juce_graphics
        PUBLIC
        juce::juce_recommended_config_flags
    )
    target_compile_definitions(ember_ui_preview PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_STANDALONE_APPLICATION=1
    )
    add_custom_target(render_panel
        COMMAND ember_ui_preview ${CMAKE_CURRENT_BINARY_DIR}/ember175-panel.png 1800 900
        DEPENDS ember_ui_preview
        COMMENT "Rendering the EMBER 175-B front panel"
    )
endif()

endif()  # EMBER_BUILD_PLUGIN
