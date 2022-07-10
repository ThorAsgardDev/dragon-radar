
#ifndef __CENGINE_H__
#define __CENGINE_H__


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "CContext.h"


class CEngine {
	private:
		
		int m_stopPipe[2];
		pthread_t m_thread;
		
		static void *staticThreadProc(void *a_pArgs);
		void *threadProc(void);
		
		
	public:
	
		CEngine(CContext *a_pContext);
		virtual ~CEngine(void);
		
		void start(void);
		void stop(void);
		void waitForEnd(void);
		
		virtual int run(int a_stopPipe) = 0;
};


#endif
