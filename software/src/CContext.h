
#ifndef __CCONTEXT_H__
#define __CCONTEXT_H__


#include <stdlib.h>


class CGraphicEngine;
class CAudioEngine;
class CGpioEngine;
class CControlEngine;


class CContext {
	private:
		
		CGraphicEngine *m_pGraphicEngine;
		CAudioEngine *m_pAudioEngine;
		CGpioEngine *m_pGpioEngine;
		CControlEngine *m_pControlEngine;
		
		
	public:
		
		CContext(void);
		virtual ~CContext(void);
		
		void setGraphicEngine(CGraphicEngine *a_pGraphicEngine);
		CGraphicEngine *getGraphicEngine(void);
		void setAudioEngine(CAudioEngine *a_pAudioEngine);
		CAudioEngine *getAudioEngine(void);
		void setGpioEngine(CGpioEngine *a_pGpioEngine);
		CGpioEngine *getGpioEngine(void);
		void setControlEngine(CControlEngine *a_pControlEngine);
		CControlEngine *getControlEngine(void);
};


#endif
