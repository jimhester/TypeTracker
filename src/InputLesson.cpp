#include "InputLesson.h"
#include <QBoxLayout>
#include <QTextEdit>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTime>
#include "ttEdit.h"

qreal pointSize = 16;
bool useGhost = true;

InputLesson::InputLesson(const InputEvent &event, QWidget* parent) :
	QWidget(parent) ,
	m_event(event) ,
	m_input(0) ,
	m_lesson(0) ,
	m_location(0) ,
	m_ghost(0) 
{

	m_time = new QTime;
	QVBoxLayout* layout = new QVBoxLayout;
	m_lesson = new QTextEdit();
	m_input = new QTextEdit();

	layout->addWidget(m_input);
	layout->addWidget(m_lesson);

	m_input->setFontPointSize(pointSize);
	m_lesson->setTextInteractionFlags(Qt::NoTextInteraction);
	m_lesson->setFontPointSize(pointSize);
	m_lesson->setTextColor(Qt::gray);
	m_lesson->setText(m_event.str());

	m_cursor = new QTextCursor(m_lesson->document());

	connect(m_input, SIGNAL(cursorPositionChanged() ),this, SLOT(changeCursorPosition() ) );

//	m_lessonText = m_lesson->toPlainText();

	m_ghost = new InputEventGhost(m_event,this);
	layout->setDirection(QVBoxLayout::BottomToTop);
	layout->addWidget(m_ghost);
	//addGhost();
	setLayout(layout);
}

InputLesson::~InputLesson(void)
{
}

void InputLesson::keyPressEvent(QKeyEvent *event){
	QWidget::keyPressEvent(event);
	//event->ignore();
}
void InputLesson::changeCursorPosition()
{
	if(!m_ghost->isRunning()){
		m_ghost->start();
	}
	int msec_elapsed = m_time->restart();
	QTextCursor cursor = m_input->textCursor();
	int newPos = cursor.position();
	if(newPos <= m_cursor->position()){
		int diff = m_cursor->position() - newPos;
		int itr = diff;
		while(itr > 0){
			m_event.addKey(QChar(0x08),0);
			itr--;
		}
		m_event.addKey(QChar(0x08),msec_elapsed);

		cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
		cursor.removeSelectedText();
		m_cursor->movePosition(QTextCursor::End);
		m_cursor->setPosition(newPos,QTextCursor::KeepAnchor);

		setSelectedTextColor(Qt::gray,m_cursor);
	} else {
		cursor.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor);
		m_cursor->setPosition(newPos,QTextCursor::KeepAnchor);
		if(m_cursor->selectedText() == cursor.selectedText()){
			m_event.addKey(cursor.selectedText(),msec_elapsed);
			setSelectedTextColor(Qt::black,m_cursor);
		} else {
			m_event.addKey(cursor.selectedText(),msec_elapsed,false);
			setSelectedTextColor(Qt::red,m_cursor);
		}
	}
	if(m_cursor->atBlockEnd()){ //True End is newline after Block, only 1 block though
		if(m_event.isValid()){
			m_event.setDate(QDateTime::currentDateTime());
			m_manager->addInputEvent(m_event);
		}
	}
	m_cursor->clearSelection();
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
	InputEventGhost * ghost = new InputEventGhost(m_event,this);
	layout()->addWidget(ghost);
}
InputEventGhost::InputEventGhost(const InputEvent &event, QWidget* parent)
	: QWidget(parent) ,
	m_event(event) ,
	m_cursor(0) ,
	m_edit(0) ,
	m_timer(0) , 
	m_location(0) ,
	m_running(false)
{
	QVBoxLayout *layout = new QVBoxLayout();

	m_edit = new QTextEdit(this);
	m_edit->setTextInteractionFlags(Qt::NoTextInteraction);

	m_cursor = new QTextCursor(m_edit->document());

	QTextCharFormat fmt;
	fmt.setFontPointSize(pointSize);
	m_cursor->mergeCharFormat(fmt);

	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()),this,SLOT(nextKey()));

	m_times = m_event.times();
	m_text = m_event.keys();

	layout->addWidget(m_edit);
	layout->setMargin(0);
	setLayout(layout);
	m_edit->show();
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
	m_timer->start(m_times[m_location]);
	m_running = true;
}
void InputEventGhost::stop()
{
	m_timer->stop();
	m_location=0;
	m_running = false;
}
void InputEventGhost::pause()
{
	m_timer->stop();
	m_running = false;
}
bool InputEventGhost::isRunning() const
{
	return m_running;
}