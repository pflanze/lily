#include <unistd.h>

#include "gReader.h"

GReader::~GReader() {}

void GReader::on_activated(int socket) {
	// XX check socket  compare
	
	notify_in->setEnabled(false);

	// The Q3Socket class provides a buffered TCP connection.
	//  oh TCP??
	// Warning: Q3Socket is not suitable for use in threads. If you need to uses sockets in threads use the lower-level Q3SocketDevice class.
	// Q3Socket s();
	// s.setSocket(socket);
	// // virtual bool 	canReadLine() const
	// QByteArray line= s.readLine(/* qint64 maxSize = 0 */);
	// oh *fun* can't get Q3Socket header file. some optional modules?

	// std::string line;
	// std::getline(std::cin, line); // XX gah get socket not cin
	// if (std::cin.eof()) {
	// 	WARN("cin is EOF");
	// } else {
	// 	WARN("GReader::on_activated("<< socket<<") = <"<<line <<">");
	// 	notify_in->setEnabled(true);
	// }

	// ^ Blocks of course. So: (is this going to end up with
	// select?  Well, non block please right?) (f' tired so not
	// gonna use std stuff anymore, don't want multi-layered
	// buffering anyway so if I am going to buffer then better be
	// it minimal)

	#define BUFSIZE 4096
	char cbuf[BUFSIZE];
	auto n= read(socket, cbuf, BUFSIZE);
	if (n < 0) {
		switch (errno) {
#if !(EAGAIN==EWOULDBLOCK)
#  error "EAGAIN != EWOULDBLOCK"
#endif
		case EWOULDBLOCK: {
			WARN("would block");
			notify_in->setEnabled(true);
			break;
		}
		case EINTR: {
			WARN("EINTR");
			notify_in->setEnabled(true);
			break;
		}
		default: {
			WARN("IO ERROR: "<< errno); //XX make nice, exn?
		}
		}
	} else if (n == 0) {
		WARN("EOF on filehandle " << socket);
		// disconnect(); or how to close it? But anyway, this
		// code is only called once, thankfully.
		_processClose();
	} else {
		bool gotline=false;
		std::string line;
		
		for (int i=0; i<n; i++) {
			char c= cbuf[i];
			if (c == '\n') {
				line= buf;
				gotline=true;
				buf="";
			} else {
				buf.push_back(c);
			}
		}
		if (gotline) {
			_processLine(line);
		}
	 	notify_in->setEnabled(true);
	}
}
