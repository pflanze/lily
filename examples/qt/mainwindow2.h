#ifndef MAINWINDOW2_H
#define MAINWINDOW2_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QWebView>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void saveFile();

    // instead move code that accesses the tabs in here?
    QWidget* maybeGetTab(std::string name);
    void setHtml(const QString html);
    
public slots:
    // menu
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionEval_triggered();
    void on_actionQuit_triggered();
    // tabs
    void on_t_log_triggered();
    void on_t_log_triggered_auto();
    void on_tab_currentChanged(int i);

    void setLog(QString s);
    void LOG(QString s);

private slots:
    // buttons (created private by qtcreator, unlike menu slots)
    void on_evalButton_clicked();
    void on_copyLogButton_clicked();
    void on_deleteLogButton_clicked();

protected:
    void setLogClean();
    void setLogDirty();
    void setLogTextClean();
    void setLogTextDirty();

    int tabIndexOf(QWidget* w);
    void setTabText(QWidget* w, QString s);
    
private:
    bool logTabDirty;
    QString logCurrentDate;
    Ui::MainWindow *ui;
    QString filepath;
    QWebView* t_view_htmlview;
    int log_autoevent_count;

    void ui_log_append(QString s);
};

#endif // MAINWINDOW2_H
