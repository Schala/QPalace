#ifndef _IPT_MANAGER_H
#define _IPT_MANAGER_H

#include <exception>
#include <QByteArray>
#include <QException>
#include <QStack>

class QPScriptError final: public QException {
public:
	QPScriptError(std::exception &e, QByteArray &msg, int offs = -1):
		mException(e), mMsg(msg), mOffs(offs) {}
		
	inline const char* what() const {
		return mMsg.data();
	}
	
	inline const int offset const {
		return mOffs;
	}
	
	inline std::exception& err() const {
		return mException;
	}
private:
	int mOffs;
	QByteArray mMsg;
	std::exception mException;
};

class QPScriptRunnable {
public:
	virtual void execute(QPScriptContextPtr ctxt) = 0;
	virtual void step() = 0;
	virtual void end() = 0;
	virtual bool isRunning() const = 0;
	virtual QByteArray& toString() = 0;
};

class QPScriptManager {
public:
	QStack<QPScriptRunnable*> callStack;
	
	QPScriptManager():
		mPaused(false), mDebug(false), mStepsPerTimeSlice(800), mTimeSliceInterval(1),
		mStepThru(false), mRunning(false), mGlobals(new QPScriptVarCache(new QPScriptContext(this))),
		mParser(new QPScriptParser(this)) {};
	virtual ~QPScriptManager();
	
	inline bool isRunning() const {
		return mRunning;
	};
private:
	QStack<QPScriptAlarm*> mAlarmStack;
	QPScriptParser *mParser;
	QPScriptVarCachePtr mGlobals;
	QByteArray mCurrentScript;
	bool mPaused, mDebug, mStepThru, mRunning;
	uint mStepsPerTimeSlice, mTimeSliceInterval;
};

#endif // _IPT_MANAGER_H
