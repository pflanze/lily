
#include <QPushButton>
#include <QPushButton>

#include "gutil.h"


// Why do I have to do this myself, nothing linked in docs.  (Use
// QIODevice::errorString() for actual OS error, this one is for
// providing the "location" (usage) of the OS api.)
const char*
fileErrorToMessage(QFile::FileError err) {
	switch (err) {
	case QFile::NoError:
		return "No error occurred";
	case QFile::ReadError:
		return "An error occurred when reading from the file";
	case QFile::WriteError:
		return "An error occurred when writing to the file";
	case QFile::FatalError:
		return "A fatal error occurred";
	case QFile::ResourceError:
		return "ResourceError";
	case QFile::OpenError:
		return "The file could not be opened";
	case QFile::AbortError:
		return "The operation was aborted";
	case QFile::TimeOutError:
		return "A timeout occurred";
	case QFile::UnspecifiedError:
		return "An unspecified error occurred";
	case QFile::RemoveError:
		return "The file could not be removed";
	case QFile::RenameError:
		return "The file could not be renamed";
	case QFile::PositionError:
		return "The position in the file could not be changed";
	case QFile::ResizeError:
		return "The file could not be resized";
	case QFile::PermissionsError:
		return "The file could not be accessed";
	case QFile::CopyError:
		return "The file could not be copied";
	default:
		throw std::logic_error(STR("unknown QFile::FileError value:" << (int)err));
		//XX avoid exceptions q? or catch them *where* ? Crossing the monads.
	}
}

// QIODevice does not have error(), only QFile
void emitErrorMessage(QFile& f) {
	auto path= f.fileName();

	QString html=
		makeBoldP(QString(fileErrorToMessage(f.error())).append(": ").append
			  (f.errorString())).append
		(makeP(toQString(lily::show(path.toStdString()))));
	errormessagedialog->showMessage(html);
}


// escape, and use pstart and pend around each paragraph for
// formatting instructions.
QString makeHtml(QString s, QString pstart, QString pend) {
	auto appStart= [&](QString s) { return s.append(pstart); };
	auto prepEnd= [&](QString s) { return pend.append(s); };
	
	s.replace("&", "&amp;");
	s.replace("<", "&lt;");
	s.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
	// ^ find a better solution, that also works with copy pasting
	// for example (informativeText field, which only handles
	// plain text, does display the tabs and they are copy pasted
	// as such, too)
	auto parts= s.split("\n\n");
	
	s= "<p>";
	s= appStart(s)
		.append(parts.join(prepEnd(appStart("</p><p>"))))
		.append(prepEnd("</p>"));
	s.replace("\n", "<br/>");
	return s;
}

QString makeBold(QString s) {
	return makeHtml(s, "<b>", "</b>");
}

QString makeBoldP(QString s) {
	return makeHtml(s, "<p><b>", "</b></p>");
}

QString makeP(QString s) {
	return makeHtml(s, "<p>", "</p>");
}



bool askConfirmOverwrite(QFileInfo& i1) {
	QMessageBox ask;

	// go all the way and add icons to the buttons like in the
	// dialog from the file dialog; NOTE:
	// https://doc.qt.io/archives/qt-4.8/qicon.html#fromTheme says
	// non-X11 needs manual bundling of icons? ! (Fall back will
	// be no icon, right?)
	QIcon cancelicon= QIcon::fromTheme("stop"); // X?
	QIcon replaceicon= QIcon::fromTheme("document-save"); // X?
	QPushButton cancelbutton(cancelicon, tr("Cancel"), &ask);
	QPushButton replacebutton(replaceicon, tr("Replace"), &ask);
	// the order of add has no effect on default.
	ask.addButton(&replacebutton, QMessageBox::AcceptRole);
	ask.addButton(&cancelbutton, QMessageBox::RejectRole);
	ask.setDefaultButton(&replacebutton);
	ask.setEscapeButton(&cancelbutton);
	// / button icons
	// // Alternative:
	// ask.setStandardButtons
	// 	(QMessageBox::Cancel
	// 	 | QMessageBox::Yes);
	// // ^ File dialog has "Cancel" and
	// // "Replace", but there's no
	// // QMessageBox::Replace

	ask.setIcon(QMessageBox::Question);

	ask.setTextFormat(Qt::RichText);
	ask.setText
		(makeBold
		 (QSTR("A file named "
		       <<
		       lily::show(i1.fileName().toStdString())
		       <<
		       " already exists.  Do you want to replace it?")));
	QFileInfo dir(i1.dir().path());
	ask.setInformativeText
		(tr("The file with the default suffix \".scm\" appended already exists in ")
		 .append(toQString(lily::show(dir.fileName().toStdString())))
		 .append(tr(".  Replacing it will overwrite its contents.")));

	/* auto result=*/ ask.exec();
	// why have the roles explained in the docs when then not
	// getting it or w?
	auto result= ask.clickedButton(); 
	if (result == &cancelbutton)
		return false;
	else if (result == &replacebutton)
		return true;
	else
		throw std::logic_error("bug");
	return false; // what, it doesn't know about plain throw being noreturn ?
}


QMessageBox* errorDialog(QString msg, QString submsg,
			 QMessageBox::Icon icontype,
			 Qt::WindowModality modality)
{
	auto dialog= new QMessageBox();

	QPushButton* okbutton;
	if (false) {
		QIcon okicon= QIcon::fromTheme("forward"); // X?
		okbutton= new QPushButton(okicon, tr("OK"), dialog);
	} else {
		okbutton= new QPushButton(tr("OK"), dialog);
	}
	dialog->addButton(okbutton, QMessageBox::AcceptRole);
	dialog->setDefaultButton(okbutton);
	dialog->setEscapeButton(okbutton);

	dialog->setIcon(icontype);

	dialog->setWindowModality(modality);

	dialog->setTextFormat(Qt::RichText);
	dialog->setText(makeBold(msg));

	dialog->setInformativeText(submsg);

	QObject::connect(dialog, SIGNAL(finished(int)),
			 dialog, SLOT(deleteLater()));
	// XX this shouldn't be necessary, right?: (anyway it still leaks)
	QObject::connect(dialog, SIGNAL(finished(int)),
			 okbutton, SLOT(deleteLater()));
	
	// HACK since will need to properly block in scm anyway
	if (modality == Qt::NonModal) {
		dialog->show();
		return dialog; // XX dangerous, how do I know when deleted??
	} else {
		/* auto result=*/ dialog->exec();
		// why have the roles explained in the docs when then not
		// getting it or w?
		auto result= dialog->clickedButton(); 
		if (result != okbutton)
			throw std::logic_error("bug");
		return NULL;
	}
}

