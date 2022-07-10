
#include "CEngine.h"


CEngine::CEngine(CContext *a_pContext) {
	pipe2(m_stopPipe, O_NONBLOCK);
	m_thread = pthread_self();
}

CEngine::~CEngine(void) {
	stop();
	close(m_stopPipe[0]);
	close(m_stopPipe[1]);
}

void CEngine::start(void) {
	int ret;
	
	ret = pthread_create(&m_thread, NULL, staticThreadProc, this); 
	
	if(ret != 0) {
		printf("Error CEngine::start pthread_create: %d\n", ret);
	}
}

void CEngine::stop(void) {
	if(pthread_equal(m_thread, pthread_self()) == 0) {
		char c;
		
		c = 0;
		write(m_stopPipe[1], &c, 1);
		
		waitForEnd();
	}
}

void CEngine::waitForEnd(void) {
	if(pthread_equal(m_thread, pthread_self()) == 0) {
		int ret;
		
		ret = pthread_join(m_thread, NULL);
		
		if(ret != 0 && ret != ESRCH) {
			printf("Error CEngine::stop pthread_join: %d\n", ret);
		}
		
		m_thread = pthread_self();
	}
}

void *CEngine::threadProc(void) {
	int ret;
	
	ret = run(m_stopPipe[0]);
	
	return (void *) ret;
}

void *CEngine::staticThreadProc(void *a_pArgs) {
	void *ret;
	CEngine *pEngine;
	
	ret = NULL;
	pEngine = (CEngine *)a_pArgs;
	
	if(pEngine) {
		ret = pEngine->threadProc();
	}
	
	return ret;
}
