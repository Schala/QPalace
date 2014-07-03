#ifndef _IPT_MANAGER_H
#define _IPT_MANAGER_H

#include <exception>
#include <QByteArray>
#include <QException>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QStack>
#include <QtGlobal>

class QPScriptError final: public QException
{
public:
	QPScriptError(std::exception &e, const char *msg, qint32 offs = -1):
		mException(e), mMsg(msg), mOffs(offs) {}
	inline const char* what() const { return mMsg; }
	inline const qint32 offset const { return mOffs; }
	inline std::exception& err() const { return mException; }
private:
	qint32 mOffs;
	char *mMsg;
	std::exception mException;
};

class QPScriptRunnable: public QSharedData
{
public:
	virtual void execute(QPScriptContextPtr ctxt) = 0;
	virtual void step() = 0;
	virtual void end() = 0;
	virtual bool isRunning() const = 0;
	virtual char* toString() = 0;
};

typedef QSharedDataPointer<QPScriptRunnable> QPScriptRunnablePtr;

class QPScriptManager
{
public:
	QStack<QPScriptRunnablePtr> callStack;
	
	QPScriptManager():
		mPaused(false), mDebug(false), mStepsPerTimeSlice(800), mTimeSliceInterval(1),
		mStepThru(false), mRunning(false), mGlobals(new QPScriptVarCache(new QPScriptContext(this))),
		mParser(new QPScriptParser(this)) {}
	virtual ~QPScriptManager();
	
	inline bool isRunning() const { return mRunning; }
private:
	QStack<QPScriptAlarm*> mAlarmStack;
	QPScriptParser *mParser;
	QPScriptVarCachePtr mGlobals;
	QByteArray mCurrentScript;
	bool mPaused, mDebug, mStepThru, mRunning;
	uint mStepsPerTimeSlice, mTimeSliceInterval;
};

#endif // _IPT_MANAGER_H
