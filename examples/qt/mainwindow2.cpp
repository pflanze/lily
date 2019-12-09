#include <QFileDialog>
#include <QClipboard>
#include <QDateTime>
#include <QGridLayout>
#include <QScrollBar>

#include <iostream>

#include "mainwindow2.h"
#include "ui_mainwindow2.h"

#include <lily.hpp>
#include <lilyParse.hpp>
#include <lilyConstruct.hpp>

#include "main.h"
#include "gutil.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    logTabDirty(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setLogTextClean();
    setLogClean(); // initialize state
    
    // wech, can't get qtcreator / qtdesigner to see the
    // on_tab_currentChanged slot on MainWindow, so do it manually
    // instead, "lol":
    QObject::connect(ui->tabWidget, SIGNAL(currentChanged(int)),
		     this, SLOT(on_tab_currentChanged(int)));

    // There's no clicked() on ui->log, at least it doesn't do
    // anything and qtcreator won't show it. It compiles though, so
    // much for "type checked".... but selectionChanged() triggers on
    // simple clicking, too, so actually perfect.
    QObject::connect(ui->log, SIGNAL(selectionChanged()),
		     this, SLOT(on_t_log_triggered()));
    // And probably just have to grab the scroll bars, too: (note: can
    // only use sliderReleased() since valueChanged(int) happens even
    // autogeneously, sliderMoved(int) only when user clicks *and
    // drags* (still not when using the mouse wheel))
    QObject::connect(ui->log->horizontalScrollBar(), SIGNAL(sliderReleased()),
                     this, SLOT(on_t_log_triggered()));
    QObject::connect(ui->log->verticalScrollBar(), SIGNAL(sliderReleased()),
                     this, SLOT(on_t_log_triggered()));
    // Try to handle mouse wheel as well, but this needs a separate
    // handler to try to ignore autogeneous events.
    QObject::connect(ui->log->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                     this, SLOT(on_t_log_triggered_auto()));
    QObject::connect(ui->log->verticalScrollBar(), SIGNAL(valueChanged(int)),
                     this, SLOT(on_t_log_triggered_auto()));
    

    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);

    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->insertRow(0);

    QStringList l;
    l.append("First name");
    l.append("Last name");
    l.append("Birthday");
    l.append("Address");
    l.append("Mobile No.");
    l.append("Other");
    l.append("Remarks");
    ui->tableWidget->setHorizontalHeaderLabels(l);

    t_view_htmlview = new QWebView(ui->t_view);
    QGridLayout *layout= new QGridLayout;
    layout->addWidget(t_view_htmlview);
    ui->t_view->setLayout(layout);//?
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setHtml(const QString html) {
    t_view_htmlview->setHtml(html);
}

QWidget* MainWindow::maybeGetTab(std::string name) {
	if (name == "t_table") {
		return ui->t_table;
	} else {
		WARN("unknown tab '"<< name <<"'");
		return NULL;
	}
}

int MainWindow::tabIndexOf(QWidget* w) {
	// XX throw exceptions for errors, please?
	return ui->tabWidget->indexOf(w);
}

void MainWindow::setTabText(QWidget* w, QString s) {
	ui->tabWidget->setTabText(tabIndexOf(w), s);
}

static auto s_log= QString("Log");
static auto s_logs= QString("Log*");

void MainWindow::setLogTextClean() {
	setTabText(ui->t_log, s_log);
}

void MainWindow::setLogTextDirty() {
	setTabText(ui->t_log, s_logs);
}


void MainWindow::setLog(QString s) {
	ui->log->setPlainText(s);
}

void MainWindow::setLogClean() {
	if (logTabDirty) {
		logTabDirty= false;
		// ^ do this first to avoid event loop (aha,
		// setLogClean is not atomic)
		
		setLogTextClean();
		// In Qt 4 at least, have to add a br tag to stop the
		// hr from being between all subsequent lines (no
		// </hr> or \n or pretty much anything else would do
		// it). Thus try to make that line as small as
		// possible. But doesn't appear to accept styles, and
		// small doesn't seem to have a (large) effect either.
		/*
		  ui_log_append(QString("<small><hr><br></small>"));
		*/

		// works except scrolls contents to the top, also
		// probably should remove previous tag!
		/*
		  ui_log_append(QString("<font color=\"gray\">")
				 .append(ui->log->toHtml())
				 .append("</font>"));
		*/

		// works:
		ui_log_append(QString("<font color=\"gray\">--------</font>"));
	}
	log_autoevent_count = 2;
	// ^ 1 reserve is necessary since first scroll event comes in
	// before the first message with the scroll bar is written out
}

void MainWindow::setLogDirty() {
	if (! logTabDirty) {
		setLogTextDirty();
		logTabDirty= true;
	}
}


void MainWindow::LOG(QString s) {
	auto t= QDateTime::currentDateTime();
	// XX is this already localtime ? ?
	auto lt= t.toLocalTime();
	// Since QDateTime doesn't provide access to year, month, day
	// as integer fields, and std lib doesn't offer thread safe
	// functions, hack on strings, ugh.
	QString date_time = lt.toString(QString("yyyy/MM/dd hh:mm:ss - "));
	auto date = date_time.left(10);
	auto time = date_time.mid(11);
	if (logCurrentDate != date) {
		// use colouring, here we can (can't append multiple
		// fragments with differing styles, i.e. no format
		// changes within a line are possible, with this Qt 4
		// version anyway)
		ui_log_append(QString("---- ").append(date).append(" ----"));
		logCurrentDate= date;
	}
	ui_log_append(time.append(s));
	setLogDirty();
}


void MainWindow::ui_log_append(QString s) {
	ui->log->append(s);
	if (ui->log->verticalScrollBar()->isVisible())
		log_autoevent_count++;	
}

void MainWindow::on_actionOpen_triggered()
{
	auto path= QFileDialog::getOpenFileName
	    (this, // XX  transient parent or w  here  ? discouraged or w
	     "Select Scheme file to open",
	     "", // start dir
	     "Scheme files (*.scm)\n"
	     "Scheme or text files (*.scm *.txt)\n"
	     "Any file (*)");
	if (! path.length())
		// XX set filepath to "" or not?
		return;

	ui->statusbar->showMessage
		(QString(STR("Opening file '"<<filepath.toStdString()<<"'...").c_str()));

	QFile f(path);
	if (! f.open(QIODevice::ReadOnly))
		goto error;

	{// to avoid compiler *error* about jumping over s even though not used there
		auto s= f.readAll();
		// "This function has no way of reporting errors; returning an
		// empty QByteArray() can mean either that no data was
		// currently available for reading, or that an error
		// occurred."
		// if (s.length() != f.size()) // sensible approach? sgh---ah nope unicode
		// 	goto error;
		// XX open problem. Move to C++11 lib or C++17?
		f.close();
		
		ui->plainTextEdit->setPlainText(s);
		ui->statusbar->showMessage
			(QString(STR("Opened '"<<path.toStdString()<<"'.").c_str()));
		// *now* it's time to confirm that we've got this file open.
		filepath= path;
	}
	return;
error:
	emitErrorMessage(f);
	// filepath= QString(""); // ugly state machine
}

void MainWindow::saveFile() {
    ui->statusbar->showMessage
	    (QString(STR("Saving to '"<<filepath.toStdString()<<"'...").c_str()));
    std::cout << "Save file: '"<<filepath.toStdString()<<"'"<<std::endl;

    auto contents= ui->plainTextEdit->toPlainText().toUtf8();
    
    // XX use tmpfile; or at least make backup!
    QFile f(filepath);
    if (! f.open(QIODevice::WriteOnly))
	    goto error;
    if (! (f.write(contents) == contents.length()))
	    goto error;
    f.close(); // XX how does that not report errors???

    ui->statusbar->showMessage
	    (QString(STR("File '"<<filepath.toStdString()<<"' saved.").c_str()));
    return;
error:
    emitErrorMessage(f);
    filepath= QString(""); // ugly state machine
}

void MainWindow::on_actionSave_triggered()
{
    if (! filepath.length()) {
	    on_actionSave_As_triggered();
    } else {
	    saveFile();
    }
}


void MainWindow::on_actionSave_As_triggered() {
	QString startpath;
redo:
        auto path= QFileDialog::getSaveFileName
            (this, // XX  transient parent or w  here  ? discouraged or w
             "Choose path to save as",
             startpath, // start dir
             "Scheme files (*.scm)\n"
             "Scheme or text files (*.scm *.txt)\n"
             "Any file (*)");
        if (path.length()) {
		QFileInfo i(path);
		// auto-complete with suffix if it doesn't have one,
		// OK?  Allow suffix-less paths if the file really
		// exists (may have been selected instead of typed)?
		if (i.suffix().length() || i.exists()) {
			filepath= path;
		} else {
			auto p= path.append(".scm"); // configure suffix?
			QFileInfo i1(p);
			if (i1.exists())
				if (! askConfirmOverwrite(i1)) {
					startpath= path;
					goto redo;
				}
			filepath= p;
		}
		saveFile();
	} else {
		goto cancelled;
        }
	return;
cancelled:
	ui->statusbar->showMessage(QString(STR("cancelled.").c_str()));
	// clear file path, OK?
	filepath= QString("");
}





void MainWindow::on_actionEval_triggered()
{
	on_evalButton_clicked();
}

void MainWindow::on_actionQuit_triggered()
{
	// XX check for unsaved changes?
	close();
}

void MainWindow::on_evalButton_clicked()
{
    QString txt= ui->plainTextEdit->toPlainText();
    LilyObjectPtr parsed= lilyParse(txt.toStdString());
    // XX check for parse errors and somehow point to their location?
    std::string result;
    bool success=true;
    LILY_TRY({
		    result= lily::show(lily::eval(parsed, environment));
	    }, {
		    success= false;
		    result= e.what();
	    })
    QString qs (result.c_str());
    if (success) {
	    ui->output->setPlainText(qs);
	    ui->statusbar->clearMessage();
    }
    else
        ui->statusbar->showMessage(qs);
}

void MainWindow::on_copyLogButton_clicked() {
	auto t= ui->log->toPlainText();
	// needed to be able to paste within Qt (but, on novo, does
	// not paste in any app outside)
	QApplication::clipboard()->setText(t, QClipboard::Clipboard);
	// needed for other apps to get it (just this makes it
	// available both in ctl-y and middle-click in emacs)
	QApplication::clipboard()->setText(t, QClipboard::Selection);
	setLogClean(); // seen it!
}

void MainWindow::on_deleteLogButton_clicked() {
	ui->log->setPlainText(QString(""));
	logCurrentDate= "";
	setLogClean();
}

void MainWindow::on_t_log_triggered() {
	setLogClean();
}

void MainWindow::on_t_log_triggered_auto() {
	log_autoevent_count--;
	if (log_autoevent_count <= 0) {
		setLogClean();
	}
}

void MainWindow::on_tab_currentChanged(int i) {
	if (i == tabIndexOf(ui->t_log))
		on_t_log_triggered();
}
