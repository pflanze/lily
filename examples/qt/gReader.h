#include <functional>
#include <iostream>

#include <QObject>
#include <QSocketNotifier>

#include "gutil.h"
// here, too? gui? well. actually yes if want to give access to the
// procs? but no, env is enough for that.

#define SOCKETTYPE int /* XX should be qintptr but where is that?*/

typedef std::function<void(std::string)> gReaderProcessLine_t;
typedef std::function<void()> gReaderProcessClose_t;

class GReader : public QObject
{
	Q_OBJECT

	SOCKETTYPE _socket;
	gReaderProcessLine_t _processLine;
	gReaderProcessClose_t _processClose;
	QSocketNotifier* notify_in;
	std::string buf;
public:
	GReader(SOCKETTYPE socket,
		gReaderProcessLine_t processLine,
		gReaderProcessClose_t processClose)
		: _socket(socket),
		  _processLine(processLine),
		  _processClose(processClose) {
		
		notify_in= new QSocketNotifier
			(socket,
			 QSocketNotifier::Read, // QSocketNotifier::Type type,
			 this);

		// XX wow lol, here they are actually using int, not
		// qintptr
		QObject::connect(notify_in, SIGNAL(activated(int)),
				 this, SLOT(on_activated(int)));

		//XX ^ move stuff to .cpp
	}
	virtual ~GReader();

public slots:
	void on_activated(int socket);
};
