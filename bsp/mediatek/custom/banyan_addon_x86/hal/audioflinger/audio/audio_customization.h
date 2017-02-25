/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_customization.h
 *
 * Project:
 * --------
 *   Android + MT6516
 *
 * Description:
 * ------------
 *   This file implements Customization base function
 *
 * Author:
 * -------
 *   Chipeng chang (mtk02308)
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 *******************************************************************************/



#include <stdint.h>
#include <sys/types.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <AudioCustomizationBase.h>

namespace android {

//AudioCustomizeInterface  Interface
class AudioCustomization : public AudioCustomizationBase
{

public:
    AudioCustomization();
    ~AudioCustomization();

    // indicate a change in device connection status
    void EXT_DAC_Init(void);

    void EXT_DAC_SetPlaybackFreq(unsigned int frequency);

    void EXT_DAC_TurnOnSpeaker(unsigned int source ,unsigned int speaker);

    void EXT_DAC_TurnOffSpeaker(unsigned int source,unsigned int speaker);

    void EXT_DAC_SetVolume(unsigned int speaker,unsigned int vol);

};

};
