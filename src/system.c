#include "CatOS.h"

Event *system_event_queue;
uchar system_event_queue_start;
uchar system_event_queue_end;
uchar system_items_in_queue;

#define SYSTEM_QUEUE_SIZE (128 / sizeof(Event))

void generate_system_event(uchar event, uchar arg, void *data) {
	EI();
	
	// If the system queue is full, let some other processes run unti there is room
	while(system_items_in_queue == SYSTEM_QUEUE_SIZE)
		force_context_switch();
	
	DI();
	
	system_event_queue[system_event_queue_end].event_id = event;
	system_event_queue[system_event_queue_end].pid = process_id;
	system_event_queue[system_event_queue_end].arg = arg;
	system_event_queue[system_event_queue_end].data = data;
	
	system_event_queue_end = (system_event_queue_end + 1) % SYSTEM_QUEUE_SIZE;
	system_items_in_queue++;
	
	if(allow_interrupts)
		EI();
}

void catos_system_process() {
	Event *e;
	
	system_event_queue = system_alloc(128);
	system_event_queue_start = 0;
	system_event_queue_end = 0;
	system_items_in_queue = 0;
	
	do {
		if(system_items_in_queue == 0)
			force_context_switch();
		else {
			e = &system_event_queue[system_event_queue_start];
			
			switch(e->event_id) {
				case EV_KILL_PROCESS:
					break;
			}
		}
	} while(1);
}






