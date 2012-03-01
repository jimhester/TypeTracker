#include <QtGui>
#include <QKeyEvent>
#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QMessageBox>
#include <QtSql>
#include <qt_windows.h>
#include <windows.h>
#include <QDebug>
#include <qtablewidget.h>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QDial>
#include "typeTracker.h"
#include "ttEdit.h"
#include "ttTableView.h"
#include "edittableview.h"
#include <qwt_slider.h>

QWidget* TypeTracker::app = 0;
 
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode >= 0){
		KBDLLHOOKSTRUCT* msg = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		if(wParam == WM_KEYDOWN){
			HWND win = TypeTracker::app->winId();
			HWND currFocus = GetFocus();
			DWORD thread = GetWindowThreadProcessId(currFocus,NULL);
			if(thread == 0){
				LPARAM nParam=MAKELPARAM(0,msg->scanCode | (msg->flags >> 8));
				PostMessage(win,wParam,msg->vkCode,nParam);
			}
		}
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
 
TypeTracker::TypeTracker()
{
	setupUi(this);

	timeout_msec = 1000;
	time = new QTime;
	timeout = new QTimer;
	timeout->setSingleShot(true);
	connect(timeout, SIGNAL(timeout()),this,SLOT(endInputEvent()));

	m_manager = new InputEventManager(this);

	tab = new QTabWidget();
	setCentralWidget(tab);
	eventTable = setupTableView(QObject::tr("Input Events"));
	m_eventTree = setupTreeView(QObject::tr("Substring"));
	QWidget *test = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout;
	QwtSlider *slider = new QwtSlider(this,Qt::Horizontal,QwtSlider::BottomScale);
	slider->setScaleMaxMajor(11);
	slider->setScaleMaxMinor(0);
	layout->addWidget(m_eventTree);
	layout->addWidget(slider);
	tab->addTab(eventTable,"Events");
	textEdit = new ttEdit();
	tab->addTab(textEdit,"CurrentLesson");
	tab->addTab(test,"test");
	tab->widget(2)->setLayout(layout);
	tab->setDocumentMode(true);
	tab->setTabsClosable(true);
	tab->setMovable(true);
	this->show();
	//connect(eventTable, SIGNAL(selectionRightClicked(const QModelIndexList &)),this,SLOT(createSubstrAnalysis(const QModelIndexList &)));
	slider->setRange(-1,10,1,1);
	connect(slider, SIGNAL(valueChanged(double)),m_treeModel,SLOT(setSubstrLength(double)));
	app = this;
	if (SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, qWinAppInst(), NULL) == 0)
        qDebug() << "Hook failed for application instance" << qWinAppInst() << "with error:" << GetLastError();
	createActions();
}

QTreeView* TypeTracker::setupTreeView(const QString &title)
{
	QTreeView* view = new QTreeView();
	InputEventModel * model = m_manager->inputEventModel();
	m_treeModel = new InputEventTreeModel(model,this);
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
	proxyModel->setSourceModel(m_treeModel);
	view->setModel(proxyModel);
	view->setWindowTitle(title);
	view->setSortingEnabled(true);
	return view;
}
EditTableView* TypeTracker::setupTableView(const QString &title)
{
	EditTableView* view = new EditTableView();
	InputEventModel * model = m_manager->inputEventModel();
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
	proxyModel->setSourceModel(model);
	view->setModel(proxyModel);
    view->setWindowTitle(title);
	view->setSortingEnabled(true);
    return view;
}
void TypeTracker::keyPressEvent(QKeyEvent *event)
{
	int msec_elapsed = 0;
	if(buf.isEmpty()){
		time->start();
	} else {
		msec_elapsed = time->restart();
	}
	timeout->start(timeout_msec);
	if(!event->text().isEmpty()){
		buf.addKey(event->text(),msec_elapsed);
	}
}

void TypeTracker::endInputEvent()
{
	if(buf.isValid()){
		buf.setDate(QDateTime::currentDateTime());
		m_manager->addInputEvent(buf);
	}
	buf.clear();
}
void TypeTracker::createSubstrAnalysis(const QModelIndexList& sel)
{
	QList<InputEvent*> events;
	QHash<int, bool> rows;
	foreach(const QModelIndex & idx, sel){
		if(!rows.contains(idx.row())){
			InputEvent* evt = static_cast<InputEvent*>(idx.data(Qt::UserRole).value<void *>());
			events << evt;
			rows[idx.row()] = true;
		}
	}
	QString label = QString("Selection %1").arg(rows.size());
	QTreeView* tree = setupTreeView(label);
	tab->addTab(tree,label);
}
void TypeTracker::createActions()
{
	actionCut->setShortcuts(QKeySequence::Cut);
	actionCut->setStatusTip(tr("Cut the current selection's contents to the "
						 "clipboard"));
	connect(actionCut, SIGNAL(triggered()), this, SLOT(cut()));

	actionCopy->setShortcuts(QKeySequence::Copy);
	actionCopy->setStatusTip(tr("Copy the current selection's contents to the "
						  "clipboard"));
	connect(actionCopy, SIGNAL(triggered()), this, SLOT(copy()));

	actionPaste->setShortcuts(QKeySequence::Paste);
	actionPaste->setStatusTip(tr("Paste the clipboard's contents into the current "
						   "selection"));
	connect(actionPaste, SIGNAL(triggered()), this, SLOT(paste()));

	actionExit->setShortcuts(QKeySequence::Quit);
	actionExit->setStatusTip(tr("Exit the application"));
	connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));

	//a_remove->setShortcuts(QKeySequence::Delete);
	//a_remove->setStatusTip(tr("Remove current row from the database"));
	//connect(a_remove, SIGNAL(triggered()), this, SLOT(remove()));
}
void TypeTracker::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	menu.addAction(a_remove);
	menu.exec(event->globalPos());
}
void TypeTracker::copy()
{
	QWidget * test = tab->currentWidget();
}