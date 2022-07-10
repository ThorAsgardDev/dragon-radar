
#ifndef __CCONTROL_ENGINE_H__
#define __CCONTROL_ENGINE_H__


#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <time.h>
#include <math.h>
#include "CContext.h"
#include "CEngine.h"
#include "graphic-engine/CGraphicEngine.h"
#include "audio-engine/CAudioEngine.h"
#include "gpio-engine/CGpioEngine.h"


#define GPIO_BUTTON 16
#define DEBOUNCE_DELAY 100 // In milliseconds

#define NZOOMS 7
#define WORLD_W (SCREEN_W * NZOOMS)
#define WORLD_H (SCREEN_H * NZOOMS)

#define MARGIN_W (DRAGON_BALL_W / 2 + DRAGON_BALL_W / 4)
#define MARGIN_H (DRAGON_BALL_H / 2 + DRAGON_BALL_H / 4)

#define PI 3.1415927


typedef struct {
	int x;
	int y;
} CONTROL_ENGINE_POSITION;


class CControlEngine : public CEngine {
	private:
		
		CContext *m_pContext;
		struct timespec m_previousT;
		int m_zoom;
		CONTROL_ENGINE_POSITION m_pDragonBallPositions[7];
		
		void setZoom(int a_zoom);
		
		static void staticOnPushButton(void *a_pCallbackUserData, EGPIO_STATE a_state);
		void onPushButton(EGPIO_STATE a_state);
		
		
	public:
	
		CControlEngine(CContext *a_pContext);
		virtual ~CControlEngine(void);
		
		virtual int run(int a_stopPipe);
};


#endif
