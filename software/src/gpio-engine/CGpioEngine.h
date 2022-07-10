
#ifndef __CGPIO_ENGINE_H__
#define __CGPIO_ENGINE_H__


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "CContext.h"
#include "CEngine.h"
#include "CGpio.h"


#define GPIO_MANAGER_MAX_GPIO 32
#define GPIO_MANAGER_BCM2708_PERI_BASE 0x20000000
#define GPIO_MANAGER_GPIO_BASE (GPIO_MANAGER_BCM2708_PERI_BASE + 0x200000)
#define GPIO_MANAGER_GPIO_BLOCK_SIZE (4 * 1024)
#define GPIO_MANAGER_GPPUD (0x94 / 4)
#define GPIO_MANAGER_GPPUDCLK0 (0x98 / 4)


class CGpioEngine : public CEngine {
	private:
		
		CGpio **m_pGpio;
		struct pollfd *m_pPollFd;
		int *m_pPollToGpio;
		unsigned char m_nPollFd;
		
		void applyPullType(EGPIO_PULL_TYPE a_pullType, unsigned long a_pullBitMask);
		void delayNano(unsigned long delay);
		
		
	public:
	
		CGpioEngine(CContext *a_pContext);
		virtual ~CGpioEngine(void);
		
		virtual int run(int a_stopPipe);
		
		void configureGpio(unsigned char a_id, EGPIO_DIRECTION a_direction, EGPIO_PULL_TYPE a_pullType, bool a_notifyInitState, GPIO_STATE_CHANGED_CALLBACK a_stateChangedCallback, void *a_pCallBackUserData);
		void applyConfiguration(void);
		void unapplyConfiguration(void);
		void setOutputState(unsigned char a_id, EGPIO_STATE a_state);
};


#endif
