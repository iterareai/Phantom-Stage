cmake_minimum_required(VERSION 3.22)
project(PhantomStage VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# --- Fetch JUCE ---
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# --- Plugin target ---
juce_add_plugin(PhantomStage
    COMPANY_NAME                "PhantomStage"
    PLUGIN_MANUFACTURER_CODE    Phst
    PLUGIN_CODE                 Phsa
    FORMATS                     AU VST3
    PRODUCT_NAME                "Phantom Stage"
    BUNDLE_ID                   "com.phantomstage.plugin"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PRODUCT_COPY_STEP           TRUE
    AU_MAIN_TYPE                kAudioUnitType_Effect
)

juce_generate_juce_header(PhantomStage)

target_sources(PhantomStage
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(PhantomStage
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(PhantomStage
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
