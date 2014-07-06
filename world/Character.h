#pragma once

class EQStreamInterface;

class Character {
public:
	Character(EQStreamInterface* pStream);
	void update();
private:
	EQStreamInterface* mStream;
};