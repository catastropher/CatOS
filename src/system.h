#ifndef SYSTEM_H
#define SYSTEM_H

// Represents an event that can be given to the system process
typedef struct {
	uchar event_id;			// ID of the event
	uchar pid;				// ID of the process which generated the event
	uchar arg;				// Additional argument
	void *data;				// Address of the additional data
} Event;

void generate_system_event(uchar event, uchar arg, void *data);
void catos_system_process();

extern Event *system_event_queue;
extern uchar system_event_queue_start;
extern uchar system_event_queue_end;
extern uchar system_items_in_queue;

#define EV_KILL_PROCESS 1

#endif