
#ifndef __CGRAPHIC_ENGINE_H__
#define __CGRAPHIC_ENGINE_H__


#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "CContext.h"
#include "CEngine.h"
#include "audio-engine/CAudioEngine.h"


#define BPP 4
#define SCREEN_W 800
#define SCREEN_SL (SCREEN_W * BPP)
#define SCREEN_H 800
#define DRAGON_BALL_W 68
#define DRAGON_BALL_H 68

#define VSYNC 1


typedef struct {
	unsigned char *pData;
	int width;
	int height;
} GRAPHIC_ENGINE_IMAGE;

typedef struct {
	unsigned char *pData;
	int fbFd;
	fb_fix_screeninfo fbFInfo;
	fb_var_screeninfo fbVInfo;
	int bufferId;
} GRAPHIC_ENGINE_GFX_CTX;

typedef struct {
	int x;
	int y;
	bool visible;
} GRAPHIC_ENGINE_DRAGON_BALL_PROPERTIES;

typedef struct {
	int gridSize;
	GRAPHIC_ENGINE_DRAGON_BALL_PROPERTIES pDragonBalls[7];
} GRAPHIC_ENGINE_PROPERTIES;


class CGraphicEngine : public CEngine {
	private:
		
		CContext *m_pContext;
		GRAPHIC_ENGINE_PROPERTIES m_properties;
		pthread_mutex_t m_mutex;
		
		int createGfxCtx(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx);
		void releaseGfxCtx(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx);
		void loadImage(const char *a_pFileName, GRAPHIC_ENGINE_IMAGE *a_pImage);
		void releaseImage(GRAPHIC_ENGINE_IMAGE *a_pImage);
		long long getTime(void);
		void flipBuffer(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_bufferId);
		void drawHLine(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_y, int a_thickness);
		void drawVLine(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_x, int a_thickness);
		void drawGrid(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_zoom);
		void drawImage(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_xDst, int a_yDst, int a_xSrc, int a_ySrc, int a_wSrc, int a_hSrc, GRAPHIC_ENGINE_IMAGE *a_pImage);
		
		
	public:
	
		CGraphicEngine(CContext *a_pContext);
		virtual ~CGraphicEngine(void);
		
		virtual int run(int a_stopPipe);
		
		void setProperties(GRAPHIC_ENGINE_PROPERTIES *a_pProperties);
		void getProperties(GRAPHIC_ENGINE_PROPERTIES *a_pProperties);
};


#endif
