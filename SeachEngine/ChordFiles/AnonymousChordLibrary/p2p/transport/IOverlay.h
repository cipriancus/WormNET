/*
 *	Abstract Class IOverlay inherits from the ITransport Class... trying to simulate
 *	an interface.
 */

#ifndef IOVERLAY_H
#define IOVERLAY_H


#include "Node.h"
#include "ITransport.h"
#include <string>

class IOverlay {
public:
	virtual string getIdentifier() = 0;
	virtual ITransport* getTransport() = 0;
	virtual Node* getThisNode() = 0;

	virtual void put(string key, string value) = 0;
	virtual string get(string key) = 0;
};

#endif
