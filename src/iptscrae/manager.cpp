#include "manager.h"

IptManager::~IptManager() {
	for (IptRunnable *call: calls)
		delete call;
	calls.clear();
	for (IptAlarm *alarm: alarms)
		delete alarm;
	alarms.clear();
	delete parser;
	delete global_vars;
}

bool IptManager::isRunning() const {
	return running;
}
