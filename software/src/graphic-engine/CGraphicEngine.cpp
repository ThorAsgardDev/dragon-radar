
#include "CGraphicEngine.h"


CGraphicEngine::CGraphicEngine(CContext *a_pContext) : CEngine(a_pContext) {
	m_pContext = a_pContext;
	memset(&m_properties, 0, sizeof(GRAPHIC_ENGINE_PROPERTIES));
	pthread_mutex_init(&m_mutex, NULL);
}

CGraphicEngine::~CGraphicEngine(void) {
	pthread_mutex_destroy(&m_mutex);
}

void CGraphicEngine::setProperties(GRAPHIC_ENGINE_PROPERTIES *a_pProperties) {
	pthread_mutex_lock(&m_mutex);
	memcpy(&m_properties, a_pProperties, sizeof(GRAPHIC_ENGINE_PROPERTIES));
	pthread_mutex_unlock(&m_mutex);
}

void CGraphicEngine::getProperties(GRAPHIC_ENGINE_PROPERTIES *a_pProperties) {
	pthread_mutex_lock(&m_mutex);
	memcpy(a_pProperties, &m_properties, sizeof(GRAPHIC_ENGINE_PROPERTIES));
	pthread_mutex_unlock(&m_mutex);
}

int CGraphicEngine::createGfxCtx(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx) {
	// Open the framebuffer device in read write
	a_pGfxCtx->fbFd = open("/dev/fb0", O_RDWR);
	if(a_pGfxCtx->fbFd < 0) {
		printf("Unable to open /dev/fb0\n");
		return 1;
	}
	// Do Ioctl. Get the variable screen info.
	if(ioctl(a_pGfxCtx->fbFd, FBIOGET_VSCREENINFO, &a_pGfxCtx->fbVInfo) < 0) {
		printf("Eroor CGraphicEngine::createGfxCtx unable to retrieve variable screen info: %s\n", strerror(errno));
		close(a_pGfxCtx->fbFd);
		return 2;
	}
	// Set virtual display size double the width for double buffering
	a_pGfxCtx->fbVInfo.yoffset = 0;
	a_pGfxCtx->fbVInfo.yres_virtual = a_pGfxCtx->fbVInfo.yres * 2;
	if(ioctl(a_pGfxCtx->fbFd, FBIOPUT_VSCREENINFO, &a_pGfxCtx->fbVInfo) < 0) {
		printf("Error CGraphicEngine::createGfxCtx setting variable screen info from fb\n");
		close(a_pGfxCtx->fbFd);
		return 3;
	}
	//Do Ioctl. Retrieve fixed screen info.
	if(ioctl(a_pGfxCtx->fbFd, FBIOGET_FSCREENINFO, &a_pGfxCtx->fbFInfo) < 0) {
		printf("Error CGraphicEngine::createGfxCtx get fixed screen info failed: %s\n", strerror(errno));
		close(a_pGfxCtx->fbFd);
		return 4;
	}
	
	if(a_pGfxCtx->fbFInfo.line_length / a_pGfxCtx->fbVInfo.xres != BPP) {
		printf("Error CGraphicEngine::createGfxCtx incompatible number of bytes per pixel (%d instead of %d)\n", a_pGfxCtx->fbFInfo.line_length / a_pGfxCtx->fbVInfo.xres, BPP);
		close(a_pGfxCtx->fbFd);
		return 5;
	}
	
	// Now mmap the framebuffer.
	a_pGfxCtx->pData = (unsigned char *)mmap(NULL, a_pGfxCtx->fbFInfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, a_pGfxCtx->fbFd, 0);
	
	if(a_pGfxCtx->pData == MAP_FAILED) {
		printf("Error CGraphicEngine::createGfxCtx mmap failed\n");
		close(a_pGfxCtx->fbFd);
		return 6;
	}
	
	return 0;
}

void CGraphicEngine::releaseGfxCtx(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx) {
	munmap(a_pGfxCtx->pData, a_pGfxCtx->fbFInfo.smem_len);
	close(a_pGfxCtx->fbFd);
	a_pGfxCtx->pData = NULL;
	a_pGfxCtx->fbFd = 0;
}

void CGraphicEngine::loadImage(const char *a_pFileName, GRAPHIC_ENGINE_IMAGE *a_pImage) {
	FILE *f;
	f = fopen(a_pFileName, "rb");
	
	if(f == NULL) {
		printf("Error CGraphicEngine::loadImage %s\n", a_pFileName);
		return;
	}
	
	fseek(f, 0, SEEK_END);
	long length = ftell(f);
	fseek(f, 0, SEEK_SET);
	fread(&a_pImage->width, sizeof(int), 1, f);
	fread(&a_pImage->height, sizeof(int), 1, f);
	a_pImage->pData = new unsigned char[length - 2 * sizeof(int)];
	fread(a_pImage->pData, length - 2 * sizeof(int), 1, f);
	fclose(f);
}

void CGraphicEngine::releaseImage(GRAPHIC_ENGINE_IMAGE *a_pImage) {
	if(a_pImage->pData) {
		delete[] a_pImage->pData;
		a_pImage->pData = NULL;
	}
}

long long CGraphicEngine::getTime(void) {
	struct timeval t; 
	gettimeofday(&t, NULL);
	long long ms = t.tv_sec * 1000 + t.tv_usec / 1000;
	return ms;
}

void CGraphicEngine::flipBuffer(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_bufferId) {
	a_pGfxCtx->fbVInfo.yoffset = a_pGfxCtx->fbVInfo.yres * a_bufferId;
	if(ioctl(a_pGfxCtx->fbFd, FBIOPAN_DISPLAY, &a_pGfxCtx->fbVInfo) < 0) {
		printf("Error CGraphicEngine::flipBuffer panning display\n");
	}
	
	if(VSYNC) {
		int dummy;
		
		if(ioctl(a_pGfxCtx->fbFd, FBIO_WAITFORVSYNC, &dummy)) {
			printf("Error CGraphicEngine::flipBuffer waiting for VSYNC\n");
		}
	}
}

void CGraphicEngine::drawHLine(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_y, int a_thickness) {
	unsigned char *p = a_pGfxCtx->pData + (a_pGfxCtx->bufferId * a_pGfxCtx->fbVInfo.yres + a_y - a_thickness / 2) * a_pGfxCtx->fbFInfo.line_length;
	while(a_thickness--) {
		memset(p, 0, SCREEN_SL);
		p += a_pGfxCtx->fbFInfo.line_length;
	}
}

void CGraphicEngine::drawVLine(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_x, int a_thickness) {
	unsigned char *p = a_pGfxCtx->pData + a_pGfxCtx->bufferId * a_pGfxCtx->fbVInfo.yres * a_pGfxCtx->fbFInfo.line_length + (a_x - a_thickness / 2) * BPP;
	int len = a_thickness * BPP;
	int y = SCREEN_H;
	while(y--) {
		memset(p, 0, len);
		p += a_pGfxCtx->fbFInfo.line_length;
	}
}

void CGraphicEngine::drawGrid(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_nZones) {
	for(int i = 1; i < a_nZones; i++) {
		drawHLine(a_pGfxCtx, i * SCREEN_H / a_nZones, 3);
		drawVLine(a_pGfxCtx, i * SCREEN_W / a_nZones, 3);
	}
}

void CGraphicEngine::drawImage(GRAPHIC_ENGINE_GFX_CTX *a_pGfxCtx, int a_xDst, int a_yDst, int a_xSrc, int a_ySrc, int a_wSrc, int a_hSrc, GRAPHIC_ENGINE_IMAGE *a_pImage) {
	int srcX1 = a_xSrc;
	int srcY1 = a_ySrc;
	int srcX2 = a_xSrc + a_wSrc;
	int srcY2 = a_ySrc + a_hSrc;
	
	if(a_xDst < 0) {
		srcX1 -= a_xDst;
	}
	if(a_xDst + a_wSrc >= SCREEN_W) {
		srcX2 -= a_xDst + a_wSrc - SCREEN_W;
	}
	if(a_yDst < 0) {
		srcY1 -= a_yDst;
	}
	if(a_yDst + a_hSrc >= SCREEN_H) {
		srcY2 -= a_yDst + a_hSrc - SCREEN_H;
	}
	
	if(srcX1 >= srcX2 || srcY1 >= srcY2) {
		return;
	}
	
	if(a_xDst < 0) {
		a_xDst = 0;
	}
	
	if(a_yDst < 0) {
		a_yDst = 0;
	}
	
	unsigned char *pSrc = a_pImage->pData + srcY1 * a_pImage->width * BPP + srcX1 * BPP;
	unsigned char *pDst = a_pGfxCtx->pData + (a_pGfxCtx->bufferId * a_pGfxCtx->fbVInfo.yres + a_yDst) * a_pGfxCtx->fbFInfo.line_length + a_xDst * BPP;
	
	int yy = srcY2 - srcY1;
	while(yy--) {
		int xx = srcX2 - srcX1;
		while(xx--) {
			unsigned char a = *(pSrc + BPP - 1);
			// unsigned char a = (*(pSrc + BPP - 1) * alpha) >> 8;
			
			switch(a) {
				case 0:
					pDst += BPP;
					pSrc += BPP;
					break;
					
				case 255:
					memcpy(pDst, pSrc, BPP - 1);
					pDst += BPP;
					pSrc += BPP;
					break;
					
				default:
					// B
					*pDst = ((a * (*pSrc - *pDst)) >> 8) + *pDst;
					pDst++;
					pSrc++;
					
					// G
					*pDst = ((a * (*pSrc - *pDst)) >> 8) + *pDst;
					pDst++;
					pSrc++;
					
					// R
					*pDst = ((a * (*pSrc - *pDst)) >> 8) + *pDst;
					pDst += 2;
					pSrc += 2;
			}
		}
		
		pSrc += (a_pImage->width - (srcX2 - srcX1)) * BPP;
		pDst += a_pGfxCtx->fbFInfo.line_length - (srcX2 - srcX1) * BPP;
		
		// memcpy(pDst, pSrc, (srcX2 - srcX1) * BPP);
		// pSrc += width * BPP;
		// pDst += a_pGfxCtx->fbFInfo.line_length;
	}
}

int CGraphicEngine::run(int a_stopPipe) {
	
	struct pollfd pollFd;
	
	memset(&pollFd, 0, sizeof(struct pollfd));
	
	pollFd.fd = a_stopPipe;
	pollFd.events = POLLIN;
	pollFd.revents = 0;
	
	/*if(poll(&pollFd, 1, -1) > 0) {
		printf("CGraphicEngine stop required\n");
	}*/
	
	GRAPHIC_ENGINE_GFX_CTX gfxCtx;
	int *pBg = new int[SCREEN_SL];
	int x;
	GRAPHIC_ENGINE_IMAGE arrow;
	GRAPHIC_ENGINE_IMAGE dragonBall;
	
	createGfxCtx(&gfxCtx);
	
	for(x = 0; x < SCREEN_SL; x++) {
		pBg[x] = 0x00008407;
	}
	
	loadImage("resources/arrow.raw", &arrow);
	loadImage("resources/dragon-ball-anim.raw", &dragonBall);
	
	flipBuffer(&gfxCtx, 0);
	
	gfxCtx.bufferId = 1;
	
	int c = 0;
	long long st = getTime();
	// int zoom = 0;
	
	int frame = 0;
	long long tAnim = getTime();
	while(1) {
		if(poll(&pollFd, 1, 0) > 0) {
			printf("CGraphicEngine stop required\n");
			break;
		}
		
		int y;
		
		unsigned char *p = gfxCtx.pData + gfxCtx.bufferId * gfxCtx.fbVInfo.yres * gfxCtx.fbFInfo.line_length;
		for(y = 0; y < SCREEN_H; y++) {
			memcpy(p, pBg, SCREEN_SL);
			p += gfxCtx.fbFInfo.line_length;
		}
		
		GRAPHIC_ENGINE_PROPERTIES properties;
		
		getProperties(&properties);
		
		drawGrid(&gfxCtx, properties.gridSize);
		
		drawImage(&gfxCtx, (SCREEN_W - arrow.width) / 2, (SCREEN_H - arrow.height) / 2 - 7, 0, 0, arrow.width, arrow.height, &arrow);
		
		if(frame < 5) {
			for(int i = 0; i < 7; i++) {
				if(properties.pDragonBalls[i].visible) {
					drawImage(&gfxCtx, properties.pDragonBalls[i].x - DRAGON_BALL_W / 2, properties.pDragonBalls[i].y - DRAGON_BALL_H / 2, 0, frame * DRAGON_BALL_H, DRAGON_BALL_W, DRAGON_BALL_H, &dragonBall);
				}
			}
		}
		
		flipBuffer(&gfxCtx, gfxCtx.bufferId);
		
		if(gfxCtx.bufferId == 1) {
			gfxCtx.bufferId = 0;
		} else {
			gfxCtx.bufferId = 1;
		}
		
		long long tA = getTime();
		if(tA - tAnim >= 60) {
			tAnim = tA;
			frame++;
			
			if(frame >= 14) {
				frame = 0;
				bool playSfx = false;
				for(int i = 0; i < 7; i++) {
					if(properties.pDragonBalls[i].visible) {
						playSfx = true;
						break;
					}
				}
				if(playSfx) {
					m_pContext->getAudioEngine()->play(AUDIO_ENGINE_SFX_TYPE_DRAGON_BALL);
				}
			}
		}
		
		c++;
		long long t = getTime();
		if(t - st >= 1000) {
			printf("FPS: %d\n", c);
			st = t;
			c = 0;
			// zoom += 1;
			// if(zoom >= 7)
				// zoom = 0;
		}
	}
	
	flipBuffer(&gfxCtx, 0);
	
	releaseImage(&dragonBall);
	releaseImage(&arrow);
	
	delete[] pBg;
	
	releaseGfxCtx(&gfxCtx);
	
	return 0;
}
