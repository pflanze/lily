#include <memory>

#include <QApplication>
#include <QSocketNotifier>
#include <QInputDialog>
#include <QDateTime>
#include <QTableWidget>

#include <lilyHelper.hpp>

#include "gReader.h"
#include "mainwindow2.h"

#include "gutil.h"
#include "main.h"


using namespace lily;
using namespace lilyConstruct;

LilyListPtr environment= NIL;

QErrorMessage* errormessagedialog;


#define LILY_NEW_QOBJECTLIST(ptr) LILY_NEW_FOREIGN_VALUE(QObjectList, ptr)

static MainWindow* mainwindowp= NULL;
void gutil_LOG(std::string s) {
	if (mainwindowp) {
		mainwindowp->LOG(toQString(s));
	} else {
		std::cerr << "(gutil_LOG without mainwindow) "
			  << s << std::endl;
	}
}


int main(int argc, char **argv)
{
	QApplication app (argc, argv);

	MainWindow mainwindow;
	mainwindow.show();
	mainwindowp= &mainwindow;

	errormessagedialog= new QErrorMessage(&mainwindow);

	lily::init();
	environment= lilyDefaultEnvironment();
#define DEFPRIM_ENVIRONMENT environment

	DEFPRIM1(LOG, "LOG",
		 LilyString, s, {
			 gutil_LOG(**s);
			 return VOID;
		 });
	
	DEFPRIM1(error_message_dialog, "error-message-dialog",
		 LilyString, s, {
			 errormessagedialog->showMessage(toQString(s));
			 return VOID;
		 });
	
	DEFPRIM2(input_dialog__get_text, "input-dialog:get-text",
		 LilyString, title, LilyString, label, {
			 bool ok= false;
			 QString res= QInputDialog::getText
				 (&mainwindow,
				  toQString(title), // const QString & title,
				  toQString(label), // const QString & label,
				  QLineEdit::Normal,
				  // ^ QLineEdit::EchoMode mode = QLineEdit::Normal,
				  QString(), // const QString & text = QString(),
				  &ok // bool * ok = 0,
				  // Qt::WindowFlags flags = 0,
				  // Qt::InputMethodHints inputMethodHints = Qt::ImhNone
					 );
			 if (ok)
				 return STRING(res.toStdString());
			 else
				 return FALSE;
		 });
	
	DEFPRIM2(error_dialog, "error-dialog",
		 LilyString, msg, LilyString, submsg, {
			 auto dialogp= errorDialog(toQString(**msg),
						   toQString(**submsg));
			 return std::shared_ptr< LilyForeignPointer<QMessageBox> >
				 (new LilyForeignPointer<QMessageBox>(dialogp));
		 });
	// (error-dialog "You <br/>need \n\n\tto\n\t\theed this" "Otherwise \n\tit's <b>going </b>to ell")

	DEFPRIM1(error_dialog_close, "error-dialog.close!",
		 LilyForeignPointer<QMessageBox>, dialog, {
			 (**dialog)->close();
			 return VOID;
		 });
	
	DEFPRIM1(QMessageBox_ptr, "QMessageBox*",
		 LilyInt64, address, {
			 return std::shared_ptr< LilyForeignPointer<QMessageBox> >
				 (new LilyForeignPointer<QMessageBox>((QMessageBox*)(**address)));
		 });
	
	DEFPRIM1(localtime_string, "localtime-string",
		 LilyString, fmt,
		 {
			 auto t= QDateTime::currentDateTime();
			 // XX is this already localtime ? ?
			 auto lt= t.toLocalTime();
			 QString date_time = lt.toString(toQString(fmt));
			 return STRING(date_time.toStdString());
		 });
	// (localtime-string "yyyy/MM/dd hh:mm:ss")
	
	// give in and use camel case? Auto convert? Accept both like Nim?
	DEFPRIM2(QTableWidget_setRowCount, "QTableWidget.setRowCount",
		 LilyForeignPointer<QTableWidget>, s,
		 LilyInt64, rows,
		 {
			 (**s)->setRowCount(**rows);
			 return VOID;
		 });

	DEFPRIM2(QTableWidget_setColumnCount, "QTableWidget.setColumnCount",
		 LilyForeignPointer<QTableWidget>, s,
		 LilyInt64, cols,
		 {
			 (**s)->setColumnCount(**cols);
			 return VOID;
		 });

	DEFPRIM1(maybe_get_tab, "maybe-get-tab",
		 LilyString, nam,
		 {
			 auto r= mainwindow.maybeGetTab(**nam);
			 if (r)
				 return LILY_NEW_FOREIGN_POINTER(QWidget, r);
			 else
				 return FALSE; // "why not NIL ?"...
		 });

	DEFPRIM1(QObject_children, "QObject.children",
		 LilyForeignPointer<QObject>, s,
		 {
			 return LILY_NEW_QOBJECTLIST((**s)->children());
		 });

	DEFPRIM1(t_view_setHtml, "html-set!",
		 LilyString, s,
		 {
			 mainwindow.setHtml(toQString(**s));
			 return VOID;
		 });
	
	// Qt is pre closures
	GReader in_reader
		(0,
		 [](std::string line) {
			// XX effing copy paste from repl.scm eh cpp. 
			LilyObjectPtr expr= lilyParse(line, true); 
			LETU_AS(err, LilyParseError, expr);
			if (err) {
				std::cout << "PARSE_ERR: " << err->what() << std::endl;
			} else {
				LILY_TRY({
						LilyObjectPtr res=
							eval(expr, environment);
						WRITELN(res);
					}, {
						std::cout << "ERR: "
							  << e.what()
							  << std::endl;
					});
				std::cout << std::flush;
			}
		},
		 [&]() {
			 mainwindow.close();
		 });
	
	auto res= app.exec();
	mainwindowp= NULL; // safe as no exceptions should pass here.
	return res;
}

