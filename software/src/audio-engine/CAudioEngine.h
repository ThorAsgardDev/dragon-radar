
#ifndef __CAUDIO_ENGINE_H__
#define __CAUDIO_ENGINE_H__


#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <alsa/asoundlib.h>
#include "CContext.h"
#include "CEngine.h"


#define AUDIO_ENGINE_DEVICE "default"


typedef enum {
	AUDIO_ENGINE_SFX_TYPE_BUTTON = 0,
	AUDIO_ENGINE_SFX_TYPE_DRAGON_BALL,
	
	NAUDIO_ENGINE_SFX_TYPE
} AUDIO_ENGINE_SFX_TYPE;

typedef enum {
	AUDIO_ENGINE_OUTPUT_TYPE_PWM = 0,
	AUDIO_ENGINE_OUTPUT_TYPE_HDMI,
	
	NAUDIO_ENGINE_OUTPUT_TYPE
} AUDIO_ENGINE_OUTPUT_TYPE;

typedef struct {
	unsigned char *pData;
	unsigned long size;
} AUDIO_ENGINE_SFX;

typedef struct {
	AUDIO_ENGINE_SFX *pSfx;
	unsigned long offset;
} AUDIO_ENGINE_TRACK;


class CAudioEngine : public CEngine {
	private:
		
		AUDIO_ENGINE_TRACK m_track;
		AUDIO_ENGINE_SFX m_sfxButton;
		AUDIO_ENGINE_SFX m_sfxDragonBall;
		int m_pPlayPipe[2];
		
		int setOutput(AUDIO_ENGINE_OUTPUT_TYPE a_outputType);
		void setMasterVolume(long a_volume);
		void loadSfx(const char *a_pFileName, AUDIO_ENGINE_SFX *a_pSfx);
		void releaseSfx(AUDIO_ENGINE_SFX *a_pSfx);
		
		
	public:
	
		CAudioEngine(CContext *a_pContext);
		virtual ~CAudioEngine(void);
		
		virtual int run(int a_stopPipe);
		
		void play(AUDIO_ENGINE_SFX_TYPE a_sfxType);
};


#endif
