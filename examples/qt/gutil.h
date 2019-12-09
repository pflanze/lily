#ifndef _GUTIL_H
#define _GUTIL_H

#include <QFile>
#include <QDir> /* required for QFileInfo even if not mentioning explicitly! */
#include <QFileInfo>
#include <QErrorMessage>
#include <QMessageBox>


#include <lily.hpp>
// #include <lilyDefaultEnvironment.hpp>
#include <lilyParse.hpp>
#include <lilyConstruct.hpp>
// XX^ trim down


extern QErrorMessage* errormessagedialog;


inline
QString toQString(std::string s) {
	// XX better, and unicode.  or use QString in lily eben
	return QString(s.c_str());
}

inline
QString toQString(LilyStringPtr s) {
	return toQString(s->value());
}

// XX should accept QStrings, too, e.g. the result of tr( )
#define QSTR(e) toQString(STR(e))


// XX how does *that* work, tr is only available in class scopes when
// using the macro there or what? ??
inline QString tr(QString v) { return v; }
inline QString tr(const char* v) { return QString(v); }




// Why do I have to do this myself, nothing linked in docs.  (Use
// QIODevice::errorString() for actual OS error, this one is for
// providing the "location" (usage) of the OS api.)
const char*
fileErrorToMessage(QFile::FileError err);

// QIODevice does not have error(), only QFile
void emitErrorMessage(QFile& f);

QString makeHtml(QString s, QString pstart, QString pend);
QString makeBold(QString s);
QString makeBoldP(QString s);
QString makeP(QString s);

bool askConfirmOverwrite(QFileInfo& i1);

QMessageBox* errorDialog(QString msg, QString submsg,
			 // QMessageBox::Information
			 // QMessageBox::Warning
			 // QMessageBox::Critical
			 QMessageBox::Icon icontype= QMessageBox::Warning,
			 // Qt::NonModal
			 // Qt::WindowModal
			 // Qt::ApplicationModal
			 Qt::WindowModality modality= Qt::NonModal);


void gutil_LOG(std::string s);
// ^ to be implemented by user! Just made visible this way. HACK?

#ifdef WARN
#  undef WARN
#endif

#define WARN(e) ([&]() {				\
			std::ostringstream _STR_o;	\
			_STR_o << "WARN: "<< e;		\
			gutil_LOG(_STR_o.str());	\
		})()


// block is evaluated in the context of a try block,
// catchblock_with_e_what is a block that sees the variable 'e' set to
// the exception type which has a what() method
#define LILY_TRY(block,catchblock_with_e_what)			\
	try block						\
	catch (std::logic_error& e) catchblock_with_e_what	\
	catch (LilyErrorWithWhat& e) catchblock_with_e_what



#endif
