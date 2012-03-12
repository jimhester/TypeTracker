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
#include "EditTableView.h"
#include "InputLesson.h"
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
			if(thread == 0)
			{
				LPARAM nParam=MAKELPARAM(0,msg->scanCode | (msg->flags >> 8));
				PostMessage(win,wParam,msg->vkCode,nParam);
			}
		}
	}
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
 
TypeTracker::TypeTracker() 
	: timeout_msec(1000)
{
	setupUi(this);
	readSettings();

	time = new QTime;
	timeout = new QTimer;
	timeout->setSingleShot(true);
	connect(timeout, SIGNAL(timeout()),this,SLOT(endInputEvent()));

	m_manager = new InputEventManager(this);

	tab = new QTabWidget(this);
	setCentralWidget(tab);
	eventTable = setupTableView(QObject::tr("Input Events"));
	
	//QVBoxLayout *layout = new QVBoxLayout;
	//m_eventTree = setupTreeView(QObject::tr("Substring"));
	//QwtSlider *slider = new QwtSlider(this,Qt::Horizontal,QwtSlider::BottomScale);
	//slider->setScaleMaxMajor(11);
	//slider->setScaleMaxMinor(0);
	//slider->setRange(-1,10,1,1);
	//layout->addWidget(m_eventTree);
	//layout->addWidget(slider);
	//connect(slider, SIGNAL(valueChanged(double)),m_treeModel,SLOT(setSubstrLength(double)));

	tab->addTab(eventTable,"Events");
	tab->setDocumentMode(true);
	tab->setTabsClosable(true);
	connect(tab,SIGNAL(tabCloseRequested(int) ),this, SLOT(closeTab(int) ) );
	tab->setMovable(true);
	this->show();
	//connect(eventTable, SIGNAL(selectionRightClicked(const QModelIndexList &)),this,SLOT(createSubstrAnalysis(const QModelIndexList &)));
	
	app = this;
	if (SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, qWinAppInst(), NULL) == 0)
        qDebug() << "Hook failed for application instance" << qWinAppInst() << "with error:" << GetLastError();
	createActions();
}

ttTreeView* TypeTracker::setupTreeView(const QString &title)
{
	ttTreeView* view = new ttTreeView(this);
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
	EditTableView* view = new EditTableView(this);
	InputEventModel * model = m_manager->inputEventModel();
	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
	proxyModel->setSourceModel(model);
	proxyModel->setDynamicSortFilter(true);
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
		qDebug() << buf.date();
		m_manager->addInputEvent(buf);
	}
	buf.clear();
}

void TypeTracker::createActions()
{
	/*actionCut->setShortcuts(QKeySequence::Cut);
	actionCut->setStatusTip(tr("Cut the current selection's contents to the "
						 "clipboard"));
	connect(actionCut, SIGNAL(triggered()), this, SLOT(cut()));
*/
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

	connect(actionCreate_Lesson, SIGNAL(triggered()), this, SLOT(createLesson()));

	connect(actionCreate_Analysis, SIGNAL(triggered()), this, SLOT(createAnalysis()));

	actionClose->setShortcuts(QKeySequence::Close);
	actionClose->setStatusTip(tr("Close current tab"));
	connect(actionClose, SIGNAL(triggered()), this, SLOT(closeTab()));

	//a_remove->setShortcuts(QKeySequence::Delete);
	//a_remove->setStatusTip(tr("Remove current row from the database"));
	//connect(a_remove, SIGNAL(triggered()), this, SLOT(remove()));
}
void TypeTracker::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	menu.addAction(actionCreate_Lesson);
	menu.addAction(actionCreate_Analysis);
	menu.exec(event->globalPos());
}
void TypeTracker::createAnalysis()
{
	QList<int> rows;
	if(QAbstractItemView* view = tab->currentWidget()->findChild<QAbstractItemView*>()){
		foreach(QModelIndex idx,view->selectionModel()->selectedIndexes()){
			int row = idx.data(InputEventModel::ItemOffsetRole).toInt();
			if(!rows.contains(row)) {
				rows << row;
			}
		}
		if(!rows.isEmpty()){
			InputEventFilterModel * filter = new InputEventFilterModel(m_manager->inputEventModel(),rows,this);
			InputEventTreeModel * treeModel = new InputEventTreeModel(filter,this);
			QSortFilterProxyModel *sort = new QSortFilterProxyModel(this);
			sort->setSourceModel(treeModel);
			ttTreeView* tree = new ttTreeView(this);
			tree->setSortingEnabled(true);
			tree->setModel(sort);
			tab->addTab(tree,"Analysis");
		}
	}
}
void TypeTracker::createLesson()
{
	if(QAbstractItemView* view = tab->currentWidget()->findChild<QAbstractItemView*>()){
		QModelIndexList indexes = view->selectionModel()->selectedIndexes();
		if(!indexes.isEmpty()){
			int row = indexes.first().data(InputEventModel::ItemOffsetRole).toInt();
			InputEvent evnt = m_manager->InputEvents().at(row);
			InputLesson* lesson = new InputLesson(evnt,this);
			lesson->setManager(m_manager);
			tab->addTab(lesson,"Lesson");
		}
	}
}
void TypeTracker::closeTab(int idx)
{
	tab->widget(idx)->deleteLater();
}
void TypeTracker::writeSettings()
{
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"TypingTracker");

	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.endGroup();

	settings.beginGroup("InputEvent");
	settings.setValue("timeout", timeout_msec);
	settings.setValue("validator", InputEvent::validator().pattern());
	settings.endGroup();
}
void TypeTracker::readSettings()
{
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"TypingTracker");

	settings.beginGroup("MainWindow");
		if(settings.contains("size"))
			resize(settings.value("size", QSize(400, 400)).toSize());
		if(settings.contains("pos"))
			move(settings.value("pos", QPoint(200, 200)).toPoint());
	settings.endGroup();

	settings.beginGroup("InputEvent");
		if(settings.contains("timeout"))
			timeout_msec = settings.value("timeout", timeout_msec).toInt();
		if(settings.contains("validator"))
			InputEvent::setValidator(QRegExp(settings.value("validator").toString()));
	settings.endGroup();
}
void TypeTracker::closeEvent(QCloseEvent *event)
{
	writeSettings();
	event->accept();
}
