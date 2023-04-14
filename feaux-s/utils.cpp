#include "utils.h"

void exportProcess(const Process& src, ProcessCompat& dest) {
	dest.id = src.id;
	dest.name = src.name.c_str();
	dest.arrivalTime = src.arrivalTime;
	dest.doneTime = src.doneTime;
	dest.reqProcessorTime = src.reqProcessorTime;
	dest.processorTime = src.processorTime;
	dest.state = src.state;

	dest.numIOEvents = src.ioEvents.size();

	if (dest.numIOEvents > 0) {
		dest.ioEvents = new IOEvent[dest.numIOEvents];

		uint j = 0;
		for (auto it = src.ioEvents.begin(); it != src.ioEvents.end(); it++) {
			dest.ioEvents[j++] = *it;
		}
	} else {
		dest.ioEvents = nullptr;  // should be ignored on the other end if there are 0 IO Events, but set it to nullptr anyway for insurance
	}
}
