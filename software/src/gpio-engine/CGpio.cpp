
#include "CGpio.h"


CGpio::CGpio(unsigned char a_id, EGPIO_DIRECTION a_direction, EGPIO_PULL_TYPE a_pullType, bool a_notifyInitState, GPIO_STATE_CHANGED_CALLBACK a_stateChangedCallback, void *a_pCallBackUserData) {
	m_id = a_id;
	m_direction = a_direction;
	m_pullType = a_pullType;
	m_notifyInitState = a_notifyInitState;
	m_stateChangedCallback = a_stateChangedCallback;
	m_pCallBackUserData = a_pCallBackUserData;
	
	m_fdOutput = -1;
	m_currentInputState = EGPIO_STATE_LOW;
}

CGpio::~CGpio(void) {
	
}

int CGpio::setAttribute(const char *a_pAttribute, const char *a_pValue) {
	char buffer[50];
	int fdAttribute;
	int nWritten;
	int length = strlen(a_pValue);
	
	sprintf(buffer, "/sys/class/gpio/gpio%d/%s", m_id, a_pAttribute);
	
	fdAttribute = open(buffer, O_WRONLY);
	if(fdAttribute < 0) {
		printf("CGpio::setAttribute can't open attribute file\n");
		return -1;
	}
		
	nWritten = write(fdAttribute, a_pValue, length);
	
	close(fdAttribute);
	
	if(nWritten != length) {
		printf("CGpio::setAttribute can't write attribute value\n");
		return -1;
	}
		
	return 0; // success
}

void CGpio::notifyState(EGPIO_STATE a_state) {
	if(m_stateChangedCallback != NULL) {
		m_stateChangedCallback(m_pCallBackUserData, a_state);
	}
}

bool CGpio::notificationRequired(void) {
	if(m_stateChangedCallback != NULL) {
		return true;
	}
	
	return false;
}

EGPIO_PULL_TYPE CGpio::getPullType(void) {
	return m_pullType;
}

void CGpio::setInputState(EGPIO_STATE a_state, bool a_initState)
{
	if(m_direction != EGPIO_DIRECTION_IN) {
		return;
	}
	
	m_currentInputState = a_state;
	
	if(a_initState == false || (a_initState == true && m_notifyInitState == true)) {
		notifyState(m_currentInputState);
	}
}

void CGpio::setOutputState(EGPIO_STATE a_state) {
	char value;
	
	if(m_direction != EGPIO_DIRECTION_OUT) {
		return;
	}
		
	if(m_fdOutput < 0) {
		return;
	}
		
	if(a_state == EGPIO_STATE_LOW) {
		value = '0';
	} else {
		value = '1';
	}
		
	write(m_fdOutput, &value, 1);
}

void CGpio::applyConfiguration(void) {
	setAttribute("active_low", "0");
	
	if(m_direction == EGPIO_DIRECTION_IN) {
		setAttribute("direction", "in");
		setAttribute("edge", "both");
	} else {
		char buffer[50];
		
		setAttribute("direction", "out");
		
		sprintf(buffer, "/sys/class/gpio/gpio%d/value", m_id);
		
		m_fdOutput = open(buffer, O_WRONLY);
		
		if(m_fdOutput < 0) {
			printf("CGpio::applyConfiguration can't access pin state\n");
		}
	}
}

void CGpio::unapplyConfiguration(void)
{
	if(m_direction == EGPIO_DIRECTION_OUT) {
		close(m_fdOutput);
		m_fdOutput = -1;
	}
}
