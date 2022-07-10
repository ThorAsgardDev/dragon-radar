
#include "CGpioEngine.h"


CGpioEngine::CGpioEngine(CContext *a_pContext) : CEngine(a_pContext) {
	m_pGpio = new CGpio *[GPIO_MANAGER_MAX_GPIO];
	m_pPollFd = new struct pollfd[GPIO_MANAGER_MAX_GPIO + 1]; // + 1 for the stopPipe
	m_pPollToGpio = new int[GPIO_MANAGER_MAX_GPIO];
	m_nPollFd = 0;
	
	memset(m_pGpio, 0, GPIO_MANAGER_MAX_GPIO * sizeof(CGpio *));
	memset(m_pPollFd, 0, (GPIO_MANAGER_MAX_GPIO + 1) * sizeof(struct pollfd));
	memset(m_pPollToGpio, 0, GPIO_MANAGER_MAX_GPIO * sizeof(int));
}

CGpioEngine::~CGpioEngine(void) {
	if(m_pPollFd) {
		int i;
		
		for(i = 0; i < m_nPollFd; i++) {
			close(m_pPollFd[i].fd);
		}
			
		delete[] m_pPollFd;
		m_pPollFd = NULL;
	}
	
	if(m_pPollToGpio) {
		delete[] m_pPollToGpio;
		m_pPollToGpio = NULL;
	}
	
	if(m_pGpio) {
		int i;
		
		for(i = 0; i < GPIO_MANAGER_MAX_GPIO; i++) {
			if(m_pGpio[i]) {
				delete m_pGpio[i];
				m_pGpio[i] = NULL;
			}
		}
		
		delete[] m_pGpio;
		m_pGpio = NULL;
	}
}

void CGpioEngine::configureGpio(unsigned char a_id, EGPIO_DIRECTION a_direction, EGPIO_PULL_TYPE a_pullType, bool a_notifyInitState, GPIO_STATE_CHANGED_CALLBACK a_stateChangedCallback, void *a_pCallBackUserData) {
	if(a_id >= GPIO_MANAGER_MAX_GPIO) {
		return;
	}
	
	if(m_pGpio == NULL) {
		return;
	}
	
	if(m_pGpio[a_id]) {
		delete m_pGpio[a_id];
		m_pGpio[a_id] = NULL;
	}
	
	m_pGpio[a_id] = new CGpio(a_id, a_direction, a_pullType, a_notifyInitState, a_stateChangedCallback, a_pCallBackUserData);
}

void CGpioEngine::delayNano(unsigned long delay) {
	struct timespec timeSpec;
	
	timeSpec.tv_sec = delay / 1000000000;
	timeSpec.tv_nsec = delay % 1000000000;
	
	nanosleep(&timeSpec, NULL);
}

void CGpioEngine::applyPullType(EGPIO_PULL_TYPE a_pullType, unsigned long a_pullBitMask) {
	int fdMem;
	unsigned int *pGpioMap;
	
	fdMem = open("/dev/mem", O_RDWR | O_SYNC);
	
	if(fdMem < 0) {
		printf("CGpioManager::applyPullUp Can't open /dev/mem\n");
	}
		
	pGpioMap = (unsigned int *)mmap(
		NULL,                            // Any adddress will do
		GPIO_MANAGER_GPIO_BLOCK_SIZE,    // Mapped block length
		PROT_READ | PROT_WRITE,          // Enable read+write
		MAP_SHARED,                      // Shared with other processes
		fdMem,                           // File to map
		GPIO_MANAGER_GPIO_BASE);         // -> GPIO registers
		
	close(fdMem); // Not needed after mmap()
	
	if(pGpioMap == MAP_FAILED) {
		printf("CGpioManager::applyPullUp Can't mmap\n");
	}
	
	if(a_pullType == EGPIO_PULL_TYPE_DOWN) {
		pGpioMap[GPIO_MANAGER_GPPUD] = 1; // Enable pulldown
	} else {
		pGpioMap[GPIO_MANAGER_GPPUD] = 2; // Enable pullup
	}
	
	// Need to wait 150 cycles. GPIO clock max frequency is 125Mhz so 1.2ms at most, wait 2ms
	delayNano(2000000);
	
	pGpioMap[GPIO_MANAGER_GPPUDCLK0] = a_pullBitMask; // Set pull mask
	
	// Need to wait 150 cycles. GPIO clock max frequency is 125Mhz so 1.2ms at most, wait 2ms
	delayNano(2000000);
	
	pGpioMap[GPIO_MANAGER_GPPUD] = 0; // Reset pull registers
	pGpioMap[GPIO_MANAGER_GPPUDCLK0] = 0;
	
	munmap((void *)pGpioMap, GPIO_MANAGER_GPIO_BLOCK_SIZE); // Done with GPIO mmap
}

void CGpioEngine::applyConfiguration(void) {
	int i;
	unsigned long pullDownBitMask;
	unsigned long pullUpBitMask;
	int fdExport;
	
	pullDownBitMask = 0;
	pullUpBitMask = 0;
	
	fdExport = open("/sys/class/gpio/export", O_WRONLY);
	if(fdExport < 0) {
		printf("CGpioManager::applyConfiguration can't open GPIO export file\n");
	}
	
	for(i = 0; i < GPIO_MANAGER_MAX_GPIO; i++) {
		CGpio *pGpio;
		
		pGpio = m_pGpio[i];
		
		if(pGpio != NULL) {
			char buffer[50];
			
			switch(pGpio->getPullType()) {
				case EGPIO_PULL_TYPE_DOWN:
					pullDownBitMask |= (1 << i);
					break;
					
				case EGPIO_PULL_TYPE_UP:
					pullUpBitMask |= (1 << i);
					break;
					
				default:
					break;
			}
			
			sprintf(buffer, "%d", i);
			write(fdExport, buffer, strlen(buffer));
			
			pGpio->applyConfiguration();
			
			if(pGpio->notificationRequired() == true) {
				sprintf(buffer, "/sys/class/gpio/gpio%d/value", i);
				
				m_pPollToGpio[m_nPollFd] = i;
				
				m_pPollFd[m_nPollFd].fd = open(buffer, O_RDONLY);
				
				if(m_pPollFd[m_nPollFd].fd < 0) {
					printf("CGpioManager::applyConfiguration can't access pin value\n");
				}
				
				m_pPollFd[m_nPollFd].events = POLLPRI; // Set up poll() events
				m_pPollFd[m_nPollFd].revents = 0;
				
				m_nPollFd++;
			}
		}
	}
	
	close(fdExport);
	
	applyPullType(EGPIO_PULL_TYPE_DOWN, pullDownBitMask);
	applyPullType(EGPIO_PULL_TYPE_UP, pullUpBitMask);
	
	for(i = 0; i < m_nPollFd; i++) {
		char c;
		
		// Read initial data
		lseek(m_pPollFd[i].fd, 0, SEEK_SET);
		
		if((read(m_pPollFd[i].fd, &c, 1) == 1)) {
			CGpio *pGpio;
			
			pGpio = m_pGpio[m_pPollToGpio[i]];
			
			if(c == '0') {
				pGpio->setInputState(EGPIO_STATE_LOW, true);
			} else {
				pGpio->setInputState(EGPIO_STATE_HIGH, true);
			}
		}
	}
}

void CGpioEngine::unapplyConfiguration(void) {
	int i;
	int fdUnexport;
	
	fdUnexport = open("/sys/class/gpio/unexport", O_WRONLY);
	if(fdUnexport < 0) {
		printf("CGpioManager::applyConfiguration can't open GPIO unexport file\n");
	}
	
	for(i = 0; i < GPIO_MANAGER_MAX_GPIO; i++) {
		CGpio *pGpio;
		
		pGpio = m_pGpio[i];
		
		if(pGpio != NULL) {
			char buffer[50];
			
			pGpio->unapplyConfiguration();
			
			sprintf(buffer, "%d", i);
			write(fdUnexport, buffer, strlen(buffer));
		}
	}
	
	close(fdUnexport);
}

void CGpioEngine::setOutputState(unsigned char a_id, EGPIO_STATE a_state) {
	if(a_id >= GPIO_MANAGER_MAX_GPIO) {
		return;
	}
	
	if(m_pGpio == NULL) {
		return;
	}
	
	if(m_pGpio[a_id] == NULL) {
		return;
	}
	
	m_pGpio[a_id]->setOutputState(a_state);
}

int CGpioEngine::run(int a_stopPipe) {
	
	int i;
	
	m_pPollFd[m_nPollFd].fd = a_stopPipe;
	m_pPollFd[m_nPollFd].events = POLLIN;
	m_pPollFd[m_nPollFd].revents = 0;
	
	while(true) {
		// Wait for Gpio signal (or stopPipe)
		if(poll(m_pPollFd, m_nPollFd + 1, -1) > 0) {
			if(m_pPollFd[m_nPollFd].revents) { // stopPipe signaled
				printf("CGpioEngine stop required\n");
				break;
			}
			for(i = 0; i < m_nPollFd; i++) {
				if(m_pPollFd[i].revents) {
					char c;
					CGpio *pGpio;
					
					pGpio = m_pGpio[m_pPollToGpio[i]];
					
					lseek(m_pPollFd[i].fd, 0, SEEK_SET);
					read(m_pPollFd[i].fd, &c, 1);
					
					if(c == '0')
						pGpio->setInputState(EGPIO_STATE_LOW, false);
					else
						pGpio->setInputState(EGPIO_STATE_HIGH, false);
					
					m_pPollFd[i].revents = 0; // Clear flag
				}
			}
		}
	}
	
	return 0;
}
