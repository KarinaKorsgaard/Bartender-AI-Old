//
//  chainEvent.h
//  summerparty
//
//  Created by Karina Korsgaard Jensen on 29/05/2018.
//

#ifndef chainEvent_h
#define chainEvent_h

typedef enum {
	BEGIN_LEARNING,
	LEARNING,
	THINKING,
	TRAINING,
	PLAYING, // these can have any name you want, eg STATE_CLAP etc
	HIT,
	PAUSE,
	POUR,
	RESET,
	EMPTY
} State;


class ChainEvent {
public:
	vector<double> timers;
	vector<double> durations;
	vector<State> eventName;
	vector<bool> looping;
	bool done = true;
	int eventNumber = 0;

	void addEvent(double duration, State name, bool loop = false) {
		timers.push_back(0.0);
		durations.push_back(duration);
		eventName.push_back(name);
		looping.push_back(loop);
	}

	void update() {

		if (!done) {
			timers[eventNumber] += ofGetLastFrameTime();
			if (timers[eventNumber] >= durations[eventNumber] + .3 && !looping[eventNumber]) {
				timers[eventNumber] = 0.0;
				eventNumber++;

				if (eventNumber > timers.size() - 1) {
					eventNumber = 0;
					done = true;
					setToEmpty();
				}
			}
		}
	}

	void beginEvents() {
		cout << "begin events" << endl;
		done = false;
		eventNumber = 0;
		for (int i = 0; i<timers.size(); i++) {
			timers[i] = 0.0;
		}
	}

	void setTo(State state) {
		for (int i = 0; i<timers.size(); i++) {
			timers[i] = 0.0;
		}
		for (int i = 0; i<eventName.size(); i++) {
			if (eventName[i] == state) {
				eventNumber = i;

			}
		}
	}
	void setToEmpty(int i = 0) {
		if (i == 0) {
			done = true;
			eventNumber = 0;
		}
		else {
			eventNumber = i;
			done = false;
		}
	}

	int getEvent() {
		return eventNumber;
	}
	double getTime() {
		return timers[eventNumber];
	}
	double getDuration() {
		return durations[eventNumber];
	}
	State getName() {
		if (done)return EMPTY;
		else return eventName[eventNumber];
	}
	void next() {
		eventNumber++;
		eventNumber = eventNumber%timers.size();
	}
	void back() {
		timers[eventNumber] = 0.0;
		eventNumber--;
		eventNumber = eventNumber%timers.size();
		timers[eventNumber] = 0.0;
	}

};

#endif /* chainEvent_h */
#pragma once
