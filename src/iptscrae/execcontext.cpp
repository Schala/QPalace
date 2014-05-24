#include "execcontext.h" 

IptExecContext::IptExecContext(IptManager *manager, IptTokenStack *stack, IptVarCache *cache) {
	if (this->stack == nullptr) {
		stack = new IptTokenStack();
		free_stack = true;
	} else
		free_stack = false;
	
	if (this->cache == nullptr) {
		cache = new IptVarCache(this);
		free_cache = true;
	} else
		free_cache = false;
}

IptExecContext::~IptExecContext() {
	if (free_stack)
		delete stack;
	if (free_cache)
		delete cache;
}

void IptExecContext::resetExecCtrls() {
	break_requested = return_requested = exit_requested = false;
}

IptExecContext* IptExecContext::clone() {
	free_cache = free_stack = false;
	return new IptExecContext(manager, stack, cache);
}
