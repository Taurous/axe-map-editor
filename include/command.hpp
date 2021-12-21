#pragma once

#include <iostream> //For debugging

class Command
{
public:
	Command() { std::cout << "Constructing Command: "; }
	~Command() {}

	virtual void redo() = 0;
	virtual void undo() = 0;

private:

};