
#include "CControlEngine.h"


CControlEngine::CControlEngine(CContext *a_pContext) : CEngine(a_pContext) {
	m_pContext = a_pContext;
	
	memset(&m_previousT, 0, sizeof(struct timespec));
	
	m_zoom = NZOOMS - 1;
	
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	
	lt->tm_sec = 0;
	lt->tm_min = 0;
	
	srand(mktime(lt));
	
	for(int i = 0; i < 7; i++) {
		int r = rand() % ((SCREEN_W - 2 * MARGIN_W) / 2);
		int a = rand() % 360;
		m_pDragonBallPositions[i].x = ((int)(cos(a * PI / 180) * r) + SCREEN_W / 2) * WORLD_W / SCREEN_W;
		m_pDragonBallPositions[i].y = ((int)(sin(a * PI / 180) * r) + SCREEN_W / 2) * WORLD_H / SCREEN_H;
	}
}

CControlEngine::~CControlEngine(void) {
	
}

void CControlEngine::setZoom(int a_zoom) {
	m_zoom = a_zoom;
	int viewX;
	int viewY;
	int viewW;
	int viewH;
	GRAPHIC_ENGINE_PROPERTIES properties;
	
	viewW = SCREEN_W * (m_zoom + 1);
	viewH = SCREEN_H * (m_zoom + 1);
	viewX = (WORLD_W - viewW) / 2;
	viewY = (WORLD_H - viewH) / 2;
	
	properties.gridSize = 4 + m_zoom * 2;
	
	int nClusters = 0;
	int clusterDragonBallOrigin[7];
	int nDragonBallsInCluster[7];
	int dragonBallToCluster[7];
	for(int i = 0; i < 7; i++) {
		clusterDragonBallOrigin[i] = -1;
		nDragonBallsInCluster[i] = 0;
		dragonBallToCluster[i] = -1;
	}
	
	for(int i = 0; i < 7; i++) {
		if(m_pDragonBallPositions[i].x < viewX || m_pDragonBallPositions[i].x >= viewX + viewW
		|| m_pDragonBallPositions[i].y < viewY || m_pDragonBallPositions[i].y >= viewY + viewH) {
			properties.pDragonBalls[i].x = 0;
			properties.pDragonBalls[i].y = 0;
			properties.pDragonBalls[i].visible = false;
		} else {
			
			properties.pDragonBalls[i].x = (m_pDragonBallPositions[i].x - viewX) * SCREEN_W / viewW;
			properties.pDragonBalls[i].y = (m_pDragonBallPositions[i].y - viewY) * SCREEN_H / viewH;
			properties.pDragonBalls[i].visible = true;
			
			int collisionDragonBall = -1;
			
			for(int j = 0; j < i; j++) {
				if(properties.pDragonBalls[i].x + DRAGON_BALL_W / 2 >= properties.pDragonBalls[j].x - DRAGON_BALL_W / 2
				&& properties.pDragonBalls[i].x - DRAGON_BALL_W / 2 < properties.pDragonBalls[j].x + DRAGON_BALL_W / 2
				&& properties.pDragonBalls[i].y + DRAGON_BALL_H / 2 >= properties.pDragonBalls[j].y - DRAGON_BALL_H / 2
				&& properties.pDragonBalls[i].y - DRAGON_BALL_H / 2 < properties.pDragonBalls[j].y + DRAGON_BALL_H / 2) {
					collisionDragonBall = j;
					break;
				}
			}
			
			if(collisionDragonBall == -1) {
				// No collision, create new cluster
				dragonBallToCluster[i] = nClusters;
				clusterDragonBallOrigin[nClusters] = i;
				nDragonBallsInCluster[nClusters] = 1;
				nClusters++;
			} else {
				int cluster = dragonBallToCluster[collisionDragonBall];
				dragonBallToCluster[i] = cluster;
				int position = nDragonBallsInCluster[cluster];
				nDragonBallsInCluster[cluster]++;
				int origin = clusterDragonBallOrigin[cluster];
				
				switch(position) {
					case 1:
						properties.pDragonBalls[i].x = properties.pDragonBalls[origin].x + DRAGON_BALL_W / 4;
						properties.pDragonBalls[i].y = properties.pDragonBalls[origin].y - DRAGON_BALL_H / 2;
						break;
					case 2:
						properties.pDragonBalls[i].x = properties.pDragonBalls[origin].x + DRAGON_BALL_W / 2;
						properties.pDragonBalls[i].y = properties.pDragonBalls[origin].y;
						break;
					case 3:
						properties.pDragonBalls[i].x = properties.pDragonBalls[origin].x + DRAGON_BALL_W / 4;
						properties.pDragonBalls[i].y = properties.pDragonBalls[origin].y + DRAGON_BALL_H / 2;
						break;
					case 4:
						properties.pDragonBalls[i].x = properties.pDragonBalls[origin].x - DRAGON_BALL_W / 4;
						properties.pDragonBalls[i].y = properties.pDragonBalls[origin].y + DRAGON_BALL_H / 2;
						break;
					case 5:
						properties.pDragonBalls[i].x = properties.pDragonBalls[origin].x - DRAGON_BALL_W / 2;
						properties.pDragonBalls[i].y = properties.pDragonBalls[origin].y;
						break;
					case 6:
						properties.pDragonBalls[i].x = properties.pDragonBalls[origin].x - DRAGON_BALL_W / 4;
						properties.pDragonBalls[i].y = properties.pDragonBalls[origin].y - DRAGON_BALL_H / 2;
						break;
				}
			}
		}
	}
	
	m_pContext->getGraphicEngine()->setProperties(&properties);
}

void CControlEngine::staticOnPushButton(void *a_pCallbackUserData, EGPIO_STATE a_state) {
	CControlEngine *pControlEngine = (CControlEngine *) a_pCallbackUserData;
	if(pControlEngine) {
		pControlEngine->onPushButton(a_state);
	}
}

void CControlEngine::onPushButton(EGPIO_STATE a_state) {
	if(a_state == EGPIO_STATE_LOW) {
		// Debounce
		struct timespec t;
		
		clock_gettime(CLOCK_REALTIME, &t);
		
		if((t.tv_sec * 1000 + t.tv_nsec / 1000000) - (m_previousT.tv_sec * 1000 + m_previousT.tv_nsec / 1000000) < DEBOUNCE_DELAY) {
			return;
		}
		
		memcpy(&m_previousT, &t, sizeof(struct timespec));
		
		m_zoom--;
		if(m_zoom < 0) {
			m_zoom = NZOOMS - 1;
		}
		m_pContext->getAudioEngine()->play(AUDIO_ENGINE_SFX_TYPE_BUTTON);
		setZoom(m_zoom);
	}
}

int CControlEngine::run(int a_stopPipe) {
	
	struct pollfd pollFd;
	
	memset(&pollFd, 0, sizeof(struct pollfd));
	
	pollFd.fd = a_stopPipe;
	pollFd.events = POLLIN;
	pollFd.revents = 0;
	
	setZoom(m_zoom);
	
	m_pContext->getGpioEngine()->configureGpio(GPIO_BUTTON, EGPIO_DIRECTION_IN, EGPIO_PULL_TYPE_UP, false, staticOnPushButton, this);
	m_pContext->getGpioEngine()->applyConfiguration();
	
	m_pContext->getAudioEngine()->start();
	m_pContext->getGraphicEngine()->start();
	m_pContext->getGpioEngine()->start();
	
	// for(int i = 0; i < 20; i++) {
		// m_pContext->getAudioEngine()->play(AUDIO_ENGINE_SFX_TYPE_BUTTON);
		// struct timespec timeSpec;
	
		// timeSpec.tv_sec = 50000000 / 1000000000;
		// timeSpec.tv_nsec = 50000000 % 1000000000;
		
		// nanosleep(&timeSpec, NULL);
	// }
	
	/*for(int i = 0; i < 10; i++) {
		sleep(3);
		m_zoom--;
		if(m_zoom < 0) {
			m_zoom = NZOOMS - 1;
		}
		m_pContext->getAudioEngine()->play(AUDIO_ENGINE_SFX_TYPE_BUTTON);
		setZoom(m_zoom);
	}
	
	if(poll(&pollFd, 1, 10000) > 0) {
		if(pollFd.revents) {
			printf("CControlEngine stop required\n");
		} else {
			printf("CControlEngine stop on timeout\n");
		}
	}*/
	while(true) {
		if(poll(&pollFd, 1, -1) > 0) {
			if(pollFd.revents) {
				printf("CControlEngine stop required\n");
				break;
			}
		}
	}
	
	m_pContext->getGpioEngine()->stop();
	m_pContext->getGraphicEngine()->stop();
	m_pContext->getAudioEngine()->stop();
	
	return 0;
}
