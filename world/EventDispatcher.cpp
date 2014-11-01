#include "EventDispatcher.h"

void EventDispatcher::event(const Event pEvent, Actor* pActor) {
	for (auto i : mListeners[pEvent])
		i->onEvent(pEvent, pActor);
}

void EventDispatcher::event(const Event pEvent, Character* pCharacter) {
	for (auto i : mListeners[pEvent])
		i->onEvent(pEvent, pCharacter);

}
void EventDispatcher::event(const Event pEvent, NPC* pNPC) {
	for (auto i : mListeners[pEvent])
		i->onEvent(pEvent, pNPC);
}