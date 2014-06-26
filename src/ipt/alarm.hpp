#ifndef _IPT_ALARM_H
#define _IPT_ALARM_H

class QPScriptAlarm {
public:
	QPScriptAlarm(QPScriptTokenList *script, QPScriptManager *mgr, int delay, QPScriptContext ctxt = nullptr):
		mCtxt(ctxt), mTokenCache(script), mDelay(delay(mDelay)), mTimer(0) {}
	
	inline int delay() const {
		return msToTicks(mDelay);
	};
	
	inline void delay(int value) {
		mDelay = ticksToMS(value-2);
		if (mDelay < 10)
			mDelay = 10;
	};
	
	void start();
	void stop();
private:
	QPScriptTokenListPtr mTokenCache;
	QPScriptContextPtr mCtxt;
	int mDelay, mTimer;
	bool mCompleted;
	
	inline int ticksToMS(int ticks) const {
		return qMax<int>(0, ticks) / 60 * 1000;
	};
	
	inline int msToTicks(int ms) const {
		return ms / 1000 * 60;
	};
};

#endif // _IPT_ALARM_H
