#define LOG_TAG "AudioCustomization"
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#include <audio_customization.h>

namespace android {


// ----------------------------------------------------------------------------
// AudioPolicyInterface implementation
// ----------------------------------------------------------------------------

// AudioCustomizeInterface

AudioCustomization::AudioCustomization()
{
    LOGD("AudioCustomization contructor");
}

AudioCustomization::~AudioCustomization()
{
    LOGD("AudioCustomization destructor");
}


void AudioCustomization::EXT_DAC_Init()
{
    LOGD("AudioCustomization EXT_DAC_Init");
}


void AudioCustomization::EXT_DAC_SetPlaybackFreq(unsigned int frequency)
{
    LOGD("AudioCustomization EXT_DAC_SetPlaybackFreq frequency = %d",frequency);
}

void AudioCustomization::EXT_DAC_TurnOnSpeaker(unsigned int source ,unsigned int speaker)
{
    LOGD("AudioCustomization EXT_DAC_TurnOnSpeaker source = %d speaker = %d",source,speaker);
}

void AudioCustomization::EXT_DAC_TurnOffSpeaker(unsigned int source , unsigned int speaker)
{
    LOGD("AudioCustomization EXT_DAC_TurnOffSpeaker source = %d speaker = %d",source,speaker);
}

void AudioCustomization::EXT_DAC_SetVolume(unsigned int speaker,unsigned int vol)
{
    LOGD("AudioCustomization EXT_DAC_SetVolume speaker = %d vol = %d",speaker,vol);
}

}; // namespace android
