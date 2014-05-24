#ifndef _IPT_EXEC_CONTEXT_H
#define _IPT_EXEC_CONTEXT_H

#include <QByteArray>
#include "token.h"

class IptExecContext final {
public:
	IptExecContext(IptManager *manager, IptTokenStack *stack = nullptr, IptVarCache *cache = nullptr):
		stack(stack), cache(cache), manager(manager);
	inline ~IptExecContext();
	inline void resetExecCtrls();
	inline IptExecContext* clone();
private:
	IptManager *manager;
	QByteArray data;
	IptTokenStack *stack;
	IptVarCache *cache;
	bool break_requested, return_requested, exit_requested, free_stack, free_cache;
};

#endif // _IPT_EXEC_CONTEXT_H
