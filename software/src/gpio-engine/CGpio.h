
#ifndef __CGPIO_H__
#define __CGPIO_H__


#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


typedef enum {
	EGPIO_DIRECTION_IN = 0,
	EGPIO_DIRECTION_OUT,
	
	NEGPIO_DIRECTION
} EGPIO_DIRECTION;

typedef enum {
	EGPIO_STATE_UNKNOWN = 0,
	EGPIO_STATE_LOW,
	EGPIO_STATE_HIGH,
	
	NEGPIO_STATE
} EGPIO_STATE;

typedef enum {
	EGPIO_PULL_TYPE_NONE = 0,
	EGPIO_PULL_TYPE_DOWN,
	EGPIO_PULL_TYPE_UP,
	
	NEGPIO_PULL_TYPE
} EGPIO_PULL_TYPE;


typedef void (*GPIO_STATE_CHANGED_CALLBACK)(void *a_pCallbackUserData, EGPIO_STATE a_state);


class CGpio {
	private:
	
		unsigned char m_id;
		EGPIO_DIRECTION m_direction;
		EGPIO_PULL_TYPE m_pullType;
		bool m_notifyInitState;
		GPIO_STATE_CHANGED_CALLBACK m_stateChangedCallback;
		void *m_pCallBackUserData;
		int m_fdOutput;
		
		EGPIO_STATE m_currentInputState;
		
		int setAttribute(const char *a_pAttribute, const char *a_pValue);
		void notifyState(EGPIO_STATE a_state);
		
		
	public:
	
		CGpio(unsigned char a_id, EGPIO_DIRECTION a_direction, EGPIO_PULL_TYPE a_pullType, bool a_notifyInitState, GPIO_STATE_CHANGED_CALLBACK a_stateChangedCallback, void *a_pCallBackUserData);
		virtual ~CGpio(void);
		
		void applyConfiguration(void);
		void unapplyConfiguration(void);
		bool notificationRequired(void);
		EGPIO_PULL_TYPE getPullType(void);
		void setInputState(EGPIO_STATE a_state, bool a_initState);
		void setOutputState(EGPIO_STATE a_state);
};


#endif
