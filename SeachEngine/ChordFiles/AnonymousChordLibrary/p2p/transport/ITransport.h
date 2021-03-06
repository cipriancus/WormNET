/*
 * This is an abstract class with pure virtual methods.
 */

#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <string>
#include "Node.h"
#include "Request.h"

class ITransport
{
public:
	virtual char*	sendRequest(Request *request, Node *destination) = 0;
	virtual	string	connectToTracker(const string &ip, int port, Node *n) = 0;
	int				getPort() { return port; }
	void			setPort( int p ) { port = p; }

protected:
	int port;
};

#endif
