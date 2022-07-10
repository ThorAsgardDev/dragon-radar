
#include "CContext.h"


CContext::CContext(void) {
	m_pGraphicEngine = NULL;
	m_pAudioEngine = NULL;
	m_pGpioEngine = NULL;
	m_pControlEngine = NULL;
}

CContext::~CContext(void) {

}

void CContext::setGraphicEngine(CGraphicEngine *a_pGraphicEngine) {
	m_pGraphicEngine = a_pGraphicEngine;
}

CGraphicEngine *CContext::getGraphicEngine(void) {
	return m_pGraphicEngine;
}

void CContext::setAudioEngine(CAudioEngine *a_pAudioEngine) {
	m_pAudioEngine = a_pAudioEngine;
}

CAudioEngine *CContext::getAudioEngine(void) {
	return m_pAudioEngine;
}

void CContext::setGpioEngine(CGpioEngine *a_pGpioEngine) {
	m_pGpioEngine = a_pGpioEngine;
}

CGpioEngine *CContext::getGpioEngine(void) {
	return m_pGpioEngine;
}

void CContext::setControlEngine(CControlEngine *a_pControlEngine) {
	m_pControlEngine = a_pControlEngine;
}

CControlEngine *CContext::getControlEngine(void) {
	return m_pControlEngine;
}
