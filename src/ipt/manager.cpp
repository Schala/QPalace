#include "manager.hpp"

QPScriptManager::~QPScriptManager() {
	for (auto entry: callStack)
		delete entry;
	callStack.clear();
	for (auto alarm: mAlarmStack)
		delete alarm;
	mAlarmStack.clear();
	delete mParser;
	delete mGlobals;
}
