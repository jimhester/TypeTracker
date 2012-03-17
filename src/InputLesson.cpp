#include "InputLesson.h"
#include <QBoxLayout>
#include <QTextEdit>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTime>
#include <QDockWidget>
#include <QGridLayout>
#include <QMainWindow>
#include "ttEdit.h"

qreal pointSize = 16;
bool useGhost = true;

InputLesson::InputLesson(const InputEvent &event, QWidget* parent) :
	QWidget(parent) ,
	m_baseEvent(event) ,
	m_input(0) ,
	m_lesson(0) ,
	m_location(0) ,
	m_ghost(0) ,
	m_timer(0) ,
	m_timeout(1000)
{

	m_time = new QTime;
	QVBoxLayout* layout = new QVBoxLayout;
	m_lesson = new QTextEdit(this);
	m_input = new QTextEdit(this);
	
	m_input->setFontPointSize(pointSize);

	m_timer = new QTimer;
	m_timer->setSingleShot(true);

	connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));

	layout->addWidget(m_input);
	layout->addWidget(m_lesson);

	m_lesson->setTextInteractionFlags(Qt::NoTextInteraction);
	m_lesson->setFontPointSize(pointSize);
	m_lesson->setTextColor(Qt::gray);
	m_lesson->setText(m_baseEvent.str());

	m_cursor = new QTextCursor(m_lesson->document());

	connect(m_input, SIGNAL(cursorPositionChanged() ),this, SLOT(changeCursorPosition() ) );

//	m_lessonText = m_lesson->toPlainText();

	m_ghost = new InputEventGhost(m_baseEvent,this);
	layout->setDirection(QVBoxLayout::BottomToTop);
	layout->addWidget(m_ghost);
	//addGhost();
	setLayout(layout);
}

InputLesson::~InputLesson(void)
{
	if(m_ghost)
		m_ghost->close();
}

//void InputLesson::keyPressEvent(QKeyEvent *event){
//	QWidget::keyPressEvent(event);
//	//event->ignore();
//}
void InputLesson::changeCursorPosition()
{
	int msec_elapsed = m_time->restart();
	if(!m_timer->isActive()){
		m_ghost->start();
		msec_elapsed = 0;
	}
	//qDebug() << msec_elapsed;
	QTextCursor cursor = m_input->textCursor();
	int newPos = cursor.position();
	if(newPos <= m_cursor->position()){
		int diff = m_cursor->position() - newPos;
		int itr = diff;
		//qDebug() << diff;
		while(itr > 1){
			m_inputEvent.addKey(QChar(0x08),0);
			itr--;
		}
		m_inputEvent.addKey(QChar(0x08),msec_elapsed);

		cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
		cursor.removeSelectedText();
		m_cursor->movePosition(QTextCursor::End);
		m_cursor->setPosition(newPos,QTextCursor::KeepAnchor);

		setSelectedTextColor(Qt::gray,m_cursor);
	} else {
		cursor.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor);
		m_cursor->setPosition(newPos,QTextCursor::KeepAnchor);
		if(m_cursor->selectedText() == cursor.selectedText()){
			m_inputEvent.addKey(cursor.selectedText(),msec_elapsed);
			setSelectedTextColor(Qt::black,m_cursor);
		} else {
			m_inputEvent.addKey(cursor.selectedText(),msec_elapsed,false);
			setSelectedTextColor(Qt::red,m_cursor);
		}
	}
	if(m_cursor->atBlockEnd()){ //True End is newline after Block, only 1 block though
		if(m_inputEvent.isValid()){
			m_inputEvent.setDate(QDateTime::currentDateTime());
			//qDebug() << m_inputEvent.keys();
			m_manager->addInputEvent(m_inputEvent);
			m_inputEvent.clear();
			m_ghost->stop();
		}
	}
	m_cursor->clearSelection();
	m_timer->start(m_timeout);
}
void InputLesson::setSelectedTextColor(QColor color, QTextCursor* cursor)
{
	QTextCharFormat fmt;
	fmt.setForeground(color);
	cursor->mergeCharFormat(fmt);
}
void InputLesson::setManager(InputEventManager* manager)
{
	m_manager = manager;
}

void InputLesson::addGhost()
{
	InputEventGhost * ghost = new InputEventGhost(m_inputEvent,this);
	layout()->addWidget(ghost);
}
void InputLesson::timeout()
{
	m_ghost->pause();
	m_ghost->rewind(m_timeout);
	m_timer->stop();
}
InputEventGhost::InputEventGhost(const InputEvent &event, QWidget* parent)
	: QTextEdit(parent) ,
	m_cursor(0) ,
	m_timer(0) , 
	m_location(0)
{

	setEvent(event);
	setTextInteractionFlags(Qt::NoTextInteraction);

	m_cursor = new QTextCursor(document());

	QTextCharFormat fmt;
	fmt.setFontPointSize(pointSize);
	m_cursor->mergeCharFormat(fmt);

	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()),this,SLOT(nextKey()));

	m_dock = new GhostDock(this, this);
}
InputEventGhost::~InputEventGhost()
{
	if(m_dock)
		m_dock->close();
}
void InputEventGhost::nextKey()
{
	if(m_text.at(m_location) == 0x08){
		m_cursor->deletePreviousChar();
	} else {
		m_cursor->insertText(m_text.at(m_location));
	}
	m_location++;
	if(m_location < m_times.size()){
		m_timer->start(m_times[m_location]);
	}
}
void InputEventGhost::start()
{
	if(!isComplete()){
		m_timer->start(m_times[m_location]);
	}
}
void InputEventGhost::stop()
{
	m_timer->stop();
	m_location=0;
}
void InputEventGhost::pause()
{
	m_timer->stop();
}
void InputEventGhost::rewind(int msec)
{
	if(!isActive() || isComplete())
		return;
	while(msec > 0 && m_location > 1 && m_times[m_location] < msec){
		msec -= m_times[m_location];
		if(m_text.at(m_location) == 0x08){
			m_cursor->insertText(m_text.at(m_location-1));
		} else {
			m_cursor->deletePreviousChar();
		}
		m_location--;
	}
}
bool InputEventGhost::isActive() const
{
	return m_timer->isActive();
}
bool InputEventGhost::isComplete() const
{
	return m_location >= m_text.size();
}
void InputEventGhost::setEvent(const InputEvent& event)
{
	m_event = event;
	m_times = m_event.times();
	m_text = m_event.keys();
}
const InputEvent* InputEventGhost::event() const
{
	return &m_event;
}

GhostDock::GhostDock(InputEventGhost* ghost, QWidget* parent)
	: QDockWidget(parent) ,
	m_ghost(ghost)
{
	setupUi(this);
	setAllowedAreas(Qt::RightDockWidgetArea);
	//qDebug() << QApplication::topLevelWidgets();
	foreach(QWidget* test, QApplication::topLevelWidgets()){
		if(QMainWindow* main = qobject_cast<QMainWindow*>(test)){
			main->addDockWidget(Qt::RightDockWidgetArea,this);
		}
	}
}
void GhostDock::populateComboBox(bool enabled)
{
	if(enabled){
		QList<int> idxs = InputEventManager::instance()->similarEvents(*m_ghost->event(),0.05);
		QList<InputEvent> events = InputEventManager::instance()->InputEvents();
		foreach(int idx, idxs){
			qDebug() << events.at(idx).str().left(5);
		}
	}
}

