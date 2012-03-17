#ifndef TYPETRACKER_H
#define TYPETRACKER_H

#include <QMainWindow>
#include <QAbstractEventDispatcher>
#include "InputEvent.h"
#include "ttTreeView.h"
#include "../ui_typeTracker.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

class ttEdit;
class inputEventModel;
class QKeyEvent;
class QSqlDatabase;
class ttTableView;
class QTreeView;
class QSortFilterProxyModel;
class QTabWidget;
class EditTableView;
//class QModelIndexList;

class TypeTracker : public QMainWindow, private Ui::MainWindow
{
  Q_OBJECT

  public:
    TypeTracker();
    //~TypeTracker();
    private slots:
      void endInputEvent();
    void createAnalysis();
    void createLesson();
    void closeTab(int);
    //void copy();
    //	void remove();
    //void eventTableClicked(const QModelIndex & index);
  protected:	
    //bool TypeTracker::eventFilter(QObject *object, QEvent *event);
    void keyPressEvent( QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *event);
  private:
    void createActions();
    bool createConnection();
    void getEvents();
    void readSettings();
    void writeSettings();

    EditTableView* setupTableView(const QString &title);
    ttTreeView* setupTreeView(const QString &title);

    static QWidget* app;
#ifdef Q_WS_WIN
    friend LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif

    InputEvent buf;
    QTimer* timeout;
    QTime* time;
    int timeout_msec;
    QSqlDatabase* db;

    QList<InputEvent*> evnts;

    QTabWidget *tab;
    EditTableView *eventTable;
    InputEventTreeModel* m_treeModel;
    ttTreeView *m_eventTree;
    InputEventManager* m_manager;

    QAction* a_remove;
};

#ifdef Q_WS_WIN
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif

#endif
