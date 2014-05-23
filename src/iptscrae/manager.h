#ifndef _IPT_MANAGER_H
#define _IPT_MANAGER_H

#include <QByteArray>
#include <QStack>

#define STACK_DEPTH 2048
#define RECURSION_LIMIT 256

class IptRunnable {
public:
	virtual void execute(IptExecContext *context) = 0;
	virtual void step() = 0;
	virtual void end() = 0;
	virtual bool running() = 0;
	virtual QByteArray& toString() = 0;
};

class IptManager {
public:
	IptManager():
		paused(false), debug(false), steps_per_time_slice(800), time_slice_interval(1),
		step_thru(false), _running(false), global_vars(new IptVarCache(new IptExecContext(this))),
		parser(new IptParser(this)) {};
	~IptManager();
	bool running() const {
		return _running;
	};
	
private:
	QStack<IptRunnable> calls;
	QStack<IptAlarm> alarms;
	IptParser *parser;
	IptVarCache *global_vars;
	QByteArray current_script;
	bool paused, debug, step_thru, _running;
	quint32 steps_per_time_slice, time_slice_interval;
};

#endif // _IPT_MANAGER_H
