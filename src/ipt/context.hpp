#ifndef _IPT_CONTEXT_H
#define _IPT_CONTEXT_H

#include <QSharedData>
#include <QSharedDataPointer>

class QPScriptContext final: public QSharedData {
public:
	QPScriptManager *manager;
	bool breakRequested, returnRequested, exitRequested;
	QPScriptTokenStackPtr stack;
	
	QPScriptContext(QPScriptManager *mgr, QPScriptTokenStackPtr stack = nullptr, QPScriptVarCachePtr cache = nullptr):
		stack(stack), mCache(cache), manager(mgr) {}
	
	QPScriptContext(QPScriptContext *other) {
		QPScriptContext(mgr, stack, mCache);
	}
	
	inline void resetExecControls() {
		breakRequested = returnRequested = exitRequested = false;
	};
private:
	QPScriptVarCachePtr mCache;
};

typedef QSharedDataPointer<QPScriptContext> QPScriptContextPtr;

#endif // _IPT_CONTEXT_H
