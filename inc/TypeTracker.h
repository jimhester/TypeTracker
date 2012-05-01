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

    public slots:
      void generateLessons();
      void readLessons();
      void writeLessons();
      void setHook();
      void releaseHook();
      void exportSelection();

  public:
    TypeTracker();
    ~TypeTracker();
    private slots:
      void endInputEvent();
    void createAnalysis();
    void createLesson();
    void closeTab(int);
  protected:	
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
    QTreeView* setupLessonTreeView(const QString &title);

    static QWidget* app;
#ifdef Q_WS_WIN
    friend LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
    HHOOK m_hook;
#endif

    InputEvent buf;
    QTimer* timeout;
    QTime* time;
    int timeout_msec;
    QSqlDatabase* db;

    QList<InputEvent*> evnts;

    QTabWidget *tab;
    EditTableView *eventTable;
    InputEventManager* m_manager;
    InputEventLessonModel *m_lessonModel;

    QAction* a_remove;

    lessonList m_lessons;
};

#ifdef Q_WS_WIN
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif

#endif
