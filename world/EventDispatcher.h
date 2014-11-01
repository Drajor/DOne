#pragma once

#include "Constants.h"
#include "Singleton.h"

class Actor;
class Character;
class NPC;

class EventListener {
public:
	virtual ~EventListener() {};
	virtual void onEvent(const Event pEvent, Actor* pActor) {};
	virtual void onEvent(const Event pEvent, Character* pCharacter) {};
	virtual void onEvent(const Event pEvent, NPC* pNPC) {};
};

typedef std::list<EventListener*> EventListenerList;

class EventDispatcher : public Singleton<EventDispatcher> {
private:
	friend class Singleton<EventDispatcher>;
	EventDispatcher() {};
	~EventDispatcher() {};
	EventDispatcher(EventDispatcher const&); // Do not implement.
	void operator=(EventDispatcher const&); // Do not implement.
public:
	void event(const Event pEvent, Actor* pActor);
	void event(const Event pEvent, Character* pCharacter);
	void event(const Event pEvent, NPC* pNPC);

	inline void subscribe(const Event pEvent, EventListener* pListener) { mListeners[pEvent].push_back(pListener); }
	inline void unsubscribe(const Event pEvent, EventListener* pListener) { mListeners[pEvent].remove(pListener); }
private:
	EventListenerList mListeners[Event::EventMax];
};