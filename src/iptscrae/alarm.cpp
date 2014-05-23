#include "alarm.h"

IptAlarm::IptAlarm(IptTokenList *script, IptManager *manager, int delay, IptExecContext *context) {
	if (this->context == nullptr) {
		this->context = new IptExecContext(manager);
		free_ctxt = true;
	} else
		free_ctxt = false;
}

IptAlarm::~IptAlarm() {
	if (free_ctxt)
		delete context;
}

int IptAlarm::getDelay() const {
	return msToTicks(delay);
};

void IptAlarm::setDelay(int value) {
	delay = ticksToMS(value-2);
	if (delay < 10)
		delay = 10;
}

void IptAlarm::start() {
	stop();
	timer = startTimer(delay);
}

void IptAlarm::stop() {
	if (timer != 0)
		killTimer(timer);
}

int IptAlarm::ticksToMS(int ticks) const {
	return std::max<int>(0, ticks) / 60 * 1000;
}

int IptAlarm::msToTicks(int ms) const {
	return ms / 1000 * 60;
}

void IptAlarm::timerEvent(QTimerEvent *event) override {
	completed = true;
}
