#ifndef _IPT_ALARM_H
#define _IPT_ALARM_H

#include <QtGlobal>

class QPScriptAlarm
{
public:
	QPScriptAlarm(QPScriptTokenList *script, QPScriptManager *mgr, qint32 delay_, QPScriptContext ctxt = nullptr):
		mCtxt(ctxt), mTokenCache(script), mDelay(delay(delay_)), mTimer(0) {}
	inline qint32 delay() const { return msToTicks(mDelay); }
	
	inline void setDelay(qint32 value)
	{
		mDelay = ticksToMS(value-2);
		if (mDelay < 10)
			mDelay = 10;
	}
	
	void start();
	void stop();
private:
	QPScriptTokenListPtr mTokenCache;
	QPScriptContextPtr mCtxt;
	qint32 mDelay, mTimer;
	bool mCompleted;
	
	inline qint32 ticksToMS(qint32 ticks) const { return qMax<qint32>(0, ticks) / 60 * 1000; }
	inline qint32 msToTicks(qint32 ms) const { return ms / 1000 * 60; };
};

#endif // _IPT_ALARM_H
