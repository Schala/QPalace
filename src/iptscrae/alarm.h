#ifndef _IPT_ALARM_H
#define _IPT_ALARM_H

#include <QTimerEvent>
#include <algorithm>
#include "manager.h"

class IptAlarm {
	Q_OBJECT
public:
	IptAlarm(IptTokenList *script, IptManager *manager, int delay, IptExecContext *context = nullptr):
		context(context), token_cache(script), delay(setDelay(delay)), timer(0);
	inline ~IptAlarm();
	inline int getDelay() const;
	inline void setDelay(int value);
	void start();
	inline void stop();
private:
	IptTokenList *token_cache;
	IptExecContext *context;
	int delay, timer;
	bool completed, free_ctxt;
	
	inline int ticksToMS(int ticks) const;
	inline int msToTicks(int ms) const;
	inline void timerEvent(QTimerEvent *event) override;
};

#endif // _IPT_ALARM_H
