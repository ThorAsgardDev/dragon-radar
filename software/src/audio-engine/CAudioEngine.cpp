
#include "CAudioEngine.h"


CAudioEngine::CAudioEngine(CContext *a_pContext) : CEngine(a_pContext) {
	pipe2(m_pPlayPipe, O_NONBLOCK);
	memset(&m_track, 0, sizeof(AUDIO_ENGINE_TRACK));
	
	loadSfx("resources/button.snd", &m_sfxButton);
	// loadSfx("elrad.snd", &m_sfxButton);
	loadSfx("resources/dragon-ball.snd", &m_sfxDragonBall);
	
	setOutput(AUDIO_ENGINE_OUTPUT_TYPE_PWM);
	setMasterVolume(100);
}

CAudioEngine::~CAudioEngine(void) {
	close(m_pPlayPipe[0]);
	close(m_pPlayPipe[1]);
	
	releaseSfx(&m_sfxDragonBall);
	releaseSfx(&m_sfxButton);
}

void CAudioEngine::setMasterVolume(long a_volume) {
	long min;
	long max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	
	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, AUDIO_ENGINE_DEVICE);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);
	
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, "PCM");
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
	
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_set_playback_volume_all(elem, a_volume * max / 100);
	
	snd_mixer_close(handle);
}

int CAudioEngine::setOutput(AUDIO_ENGINE_OUTPUT_TYPE a_outputType) {
	int ret;
	int value;
	
	snd_ctl_t *handle = NULL;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);
	
	snd_ctl_elem_id_set_numid(id, 3);
	
	ret = snd_ctl_open(&handle, AUDIO_ENGINE_DEVICE, 0);
	if(ret < 0) {
		printf("CAudioEngine::setOutput control %s open error: %s\n", AUDIO_ENGINE_DEVICE, snd_strerror(ret));
		return 1;
	}
	
	snd_ctl_elem_info_set_id(info, id);
	
	ret = snd_ctl_elem_info(handle, info);
	if(ret < 0) {
		printf("CAudioEngine::setOutput cannot find the given element from control %s\n", AUDIO_ENGINE_DEVICE);
		snd_ctl_close(handle);
		return 1;
	}
	
	snd_ctl_elem_info_get_id(info, id);
	snd_ctl_elem_value_set_id(control, id);
	
	if(a_outputType == AUDIO_ENGINE_OUTPUT_TYPE_PWM) {
		value = 1;
	} else {
		value = 2;
	}
	
	snd_ctl_elem_value_set_integer(control, 0, value);
	
	ret = snd_ctl_elem_write(handle, control);
	if(ret < 0) {
		printf("CAudioEngine::setOutput control %s element write error: %s\n", AUDIO_ENGINE_DEVICE, snd_strerror(ret));
		snd_ctl_close(handle);
		return 1;
	}
	
	snd_ctl_close(handle);
	handle = NULL;
	
	return 0;
}

void CAudioEngine::play(AUDIO_ENGINE_SFX_TYPE a_sfxType) {
	lseek(m_pPlayPipe[1], 0, SEEK_SET);
	write(m_pPlayPipe[1], &a_sfxType, sizeof(AUDIO_ENGINE_SFX_TYPE));
}

void CAudioEngine::loadSfx(const char *a_pFileName, AUDIO_ENGINE_SFX *a_pSfx) {
	FILE *f;
	
	memset(a_pSfx, 0, sizeof(AUDIO_ENGINE_SFX));
	
	f = fopen(a_pFileName, "rb");
	
	if(f == NULL) {
		printf("Error CAudioEngine::loadSfx %s\n", a_pFileName);
		return;
	}
	
	fseek(f, 0, SEEK_END);
	a_pSfx->size = ftell(f);
	fseek(f, 0, SEEK_SET);
	a_pSfx->pData = new unsigned char[a_pSfx->size];
	fread(a_pSfx->pData, a_pSfx->size, 1, f);
	fclose(f);
}

void CAudioEngine::releaseSfx(AUDIO_ENGINE_SFX *a_pSfx) {
	if(a_pSfx->pData) {
		delete[] a_pSfx->pData;
		a_pSfx->pData = NULL;
	}
}

int CAudioEngine::run(int a_stopPipe) {
	
	struct pollfd pPollFd[3];
	int ret;
	snd_pcm_t *pPcmHandle;
	snd_pcm_hw_params_t *pHwParams;
	unsigned int tmp;
	unsigned int rate = 44100;
	int channels = 2;
	
	// Open the PCM device in playback mode
	ret = snd_pcm_open(&pPcmHandle, AUDIO_ENGINE_DEVICE, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(ret < 0) {
		printf("Error CAudioEngine::run can't open '%s' PCM device. %s\n", AUDIO_ENGINE_DEVICE, snd_strerror(ret));
	}

	// Allocate parameters object and fill it with default values
	snd_pcm_hw_params_alloca(&pHwParams);
	snd_pcm_hw_params_any(pPcmHandle, pHwParams);

	// Set parameters
	ret = snd_pcm_hw_params_set_access(pPcmHandle, pHwParams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(ret < 0) {
		printf("Error CAudioEngine::run can't set interleaved mode. %s\n", snd_strerror(ret));
	}
	ret = snd_pcm_hw_params_set_format(pPcmHandle, pHwParams, SND_PCM_FORMAT_S16_LE);
	if(ret < 0) {
		printf("Error CAudioEngine::run can't set format. %s\n", snd_strerror(ret));
	}
	ret = snd_pcm_hw_params_set_channels(pPcmHandle, pHwParams, channels);
	if(ret < 0) {
		printf("Error CAudioEngine::run can't set channels number. %s\n", snd_strerror(ret));
	}
	ret = snd_pcm_hw_params_set_rate_near(pPcmHandle, pHwParams, &rate, 0);
	if(ret < 0) {
		printf("Error CAudioEngine::run can't set rate. %s\n", snd_strerror(ret));
	}
	
	// Write parameters
	ret = snd_pcm_hw_params(pPcmHandle, pHwParams);
	if(ret < 0) {
		printf("Error CAudioEngine::run can't set hardware parameters. %s\n", snd_strerror(ret));
	}
	
	// Resume information
	printf("PCM name: '%s'\n", snd_pcm_name(pPcmHandle));
	printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pPcmHandle)));
	snd_pcm_hw_params_get_channels(pHwParams, &tmp);
	printf("channels: %u\n", tmp);
	snd_pcm_hw_params_get_rate(pHwParams, &tmp, 0);
	printf("rate: %u bps\n", tmp);
	
	memset(pPollFd, 0, 3 * sizeof(struct pollfd));
	
	pPollFd[0].fd = a_stopPipe;
	pPollFd[0].events = POLLIN;
	pPollFd[0].revents = 0;
	
	pPollFd[1].fd = m_pPlayPipe[0];
	pPollFd[1].events = POLLIN;
	pPollFd[1].revents = 0;
	
	snd_pcm_poll_descriptors(pPcmHandle, &pPollFd[2], 1);
	
	int nPollFdToListen = 2;
	
	while(true) {
		if(poll(pPollFd, nPollFdToListen, -1) > 0) {
			if(pPollFd[0].revents) { // stopPipe signaled
				printf("CAudioEngine stop required\n");
				break;
			}
			
			if(pPollFd[1].revents) { // there is a track to play
				AUDIO_ENGINE_SFX_TYPE sfxType;
				lseek(pPollFd[1].fd, 0, SEEK_SET);
				read(pPollFd[1].fd, &sfxType, sizeof(AUDIO_ENGINE_SFX_TYPE));
				
				pPollFd[1].revents = 0;
				nPollFdToListen = 3;
				
				// Dragon ball sfx cannot stop the current track
				if(sfxType != AUDIO_ENGINE_SFX_TYPE_DRAGON_BALL || m_track.pSfx == NULL) {
					if(sfxType == AUDIO_ENGINE_SFX_TYPE_DRAGON_BALL) {
						m_track.pSfx = &m_sfxDragonBall;
					} else {
						m_track.pSfx = &m_sfxButton;
					}
					m_track.offset = 0;
				}
			}
			
			if(pPollFd[2].revents) {
				pPollFd[2].revents = 0;
				
				snd_pcm_sframes_t nFramesWritten = snd_pcm_writei(pPcmHandle, m_track.pSfx->pData + m_track.offset, (m_track.pSfx->size - m_track.offset) / (2 * channels));
				
				if(nFramesWritten == -EPIPE) {
					printf("Error CAudioEngine::run underrun occurred\n");
					snd_pcm_prepare(pPcmHandle);
				} else if (nFramesWritten < 0) {
					printf("Error CAudioEngine::run can't write to PCM device. %s\n", snd_strerror(nFramesWritten));
					printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pPcmHandle)));
				} else {
					m_track.offset += nFramesWritten * 2 * channels;
					
					if(m_track.offset >= m_track.pSfx->size) {
						m_track.pSfx = NULL;
						m_track.offset = 0;
						nPollFdToListen = 2;
					}
				}
			}
		}
	}
	
	snd_pcm_close(pPcmHandle);
	
	return 0;
}
