#ifndef TYPETRACKER_H
#define TYPETRACKER_H

#include <QMainWindow>
#include <QAbstractEventDispatcher>
#include <windows.h>
#include "inputEvent.h"
#include "ttTreeView.h"
#include "../ui_typeTracker.h"

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
	void createSubstrAnalysis(const QModelIndexList& sel);
	void createLesson();
	void closeTab(int);
	//void copy();
//	void remove();
	//void eventTableClicked(const QModelIndex & index);
protected:	
	//bool TypeTracker::eventFilter(QObject *object, QEvent *event);
	void keyPressEvent( QKeyEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
private:
	void createActions();
	bool createConnection();
	void getEvents();
	EditTableView* setupTableView(const QString &title);
	ttTreeView* setupTreeView(const QString &title);

	static QWidget* TypeTracker::app;
	friend LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

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

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif