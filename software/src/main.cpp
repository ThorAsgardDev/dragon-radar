
#include <stdio.h>
#include "CContext.h"
#include "graphic-engine/CGraphicEngine.h"
#include "audio-engine/CAudioEngine.h"
#include "gpio-engine/CGpioEngine.h"
#include "control-engine/CControlEngine.h"


int main(void) {
	
	if(geteuid() != 0)
	{
		printf("You must run this program with root privileges\n");
		return 1;
	}
	
	printf("Running dragon radar...\n");
	
	CContext *pContext;
	
	pContext = new CContext();
	
	pContext->setAudioEngine(new CAudioEngine(pContext));
	pContext->setGpioEngine(new CGpioEngine(pContext));
	pContext->setGraphicEngine(new CGraphicEngine(pContext));
	pContext->setControlEngine(new CControlEngine(pContext));
	
	pContext->getControlEngine()->start();
	pContext->getControlEngine()->waitForEnd();
	
	if(pContext->getControlEngine()) {
		delete pContext->getControlEngine();
		pContext->setControlEngine(NULL);
	}
	if(pContext->getGraphicEngine()) {
		delete pContext->getGraphicEngine();
		pContext->setGraphicEngine(NULL);
	}
	if(pContext->getGpioEngine()) {
		delete pContext->getGpioEngine();
		pContext->setGpioEngine(NULL);
	}
	if(pContext->getAudioEngine()) {
		delete pContext->getAudioEngine();
		pContext->setAudioEngine(NULL);
	}
	
	return 0;
}
