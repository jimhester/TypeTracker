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
  m_lesson(0) ,
  m_location(0) ,
  m_ghost(0) ,
  m_timer(0) ,
  m_timeout(1000)
{
  m_time = new QTime;
  QVBoxLayout* layout = new QVBoxLayout;
  m_lesson = new QTextEdit(this);

  layout->addWidget(m_lesson);

  m_lesson->setFontPointSize(pointSize);
  m_lesson->setTextColor(Qt::gray);
  m_lesson->setText(m_baseEvent.str());

  m_cursor = new QTextCursor(m_lesson->document());

  connect(m_lesson, SIGNAL(cursorPositionChanged() ),this, SLOT(changeCursorPosition() ) );

  layout->setDirection(QVBoxLayout::BottomToTop);
  
  m_ghost = new InputEventGhost(m_baseEvent,this);
  layout->addWidget(m_ghost);
  setLayout(layout);

   m_timer = new QTimer;
  m_timer->setSingleShot(true);

  connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

InputLesson::~InputLesson(void)
{
  if(m_ghost)
    m_ghost->close();
}

void InputLesson::changeCursorPosition()
{
  int msec_elapsed = m_time->restart();
  if(!m_timer->isActive()){
    m_ghost->start();
    msec_elapsed = 0;
  }
  QTextCursor cursor = m_lesson->textCursor();
  int newPos = cursor.position();
  if(newPos <= m_location){
     int diff = m_location - newPos;
    int itr = diff;
    while(itr > 1){
      m_inputEvent.addKey(QChar(0x08),0);
      itr--;
    }
    m_inputEvent.addKey(QChar(0x08),msec_elapsed);
    disconnect(m_lesson, SIGNAL(cursorPositionChanged() ),this, SLOT(changeCursorPosition() ) );
      if(diff > 1)
        cursor.setPosition(m_location,QTextCursor::KeepAnchor);
      cursor.insertText(m_baseEvent.str().mid(newPos,m_location-newPos));
      cursor.setPosition(newPos,QTextCursor::KeepAnchor);
      setSelectedTextColor(Qt::gray,&cursor);
      cursor.clearSelection();
      m_lesson->setTextCursor(cursor);
    connect(m_lesson, SIGNAL(cursorPositionChanged() ),this, SLOT(changeCursorPosition() ) );

  } else if(newPos == m_location+1 && newPos <= m_baseEvent.str().size()) {
    cursor.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor);
    QString text = cursor.selectedText();
    if(text == m_baseEvent.str().at(newPos-1)){
      m_inputEvent.addKey(text,msec_elapsed,true);
      setSelectedTextColor(Qt::black,&cursor);
    } else {
      m_inputEvent.addKey(text,msec_elapsed,false);
      if(text == " "){
        setSelectedTextColor(Qt::red,&cursor,true);
      } else{
        setSelectedTextColor(Qt::red,&cursor);
      }
    }
    cursor.setPosition(newPos+1);
    cursor.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    if(newPos >= m_baseEvent.str().size()){
      //finishEvent
      if(m_inputEvent.isValid()){
        m_inputEvent.setDate(QDateTime::currentDateTime());
        m_manager->addInputEvent(m_inputEvent);
        m_inputEvent.clear();
        m_ghost->stop();
      }
    }
  }
  m_location = newPos;
  m_timer->start(m_timeout);
}
void InputLesson::setSelectedTextColor(QColor color, QTextCursor* cursor,bool background)
{
  QTextCharFormat fmt;
  fmt.setBackground(Qt::white);
  if(background) {
    fmt.setBackground(color);
  }
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
  foreach(QWidget* test, QApplication::topLevelWidgets()){
    if(QMainWindow* main = qobject_cast<QMainWindow*>(test)){
      main->addDockWidget(Qt::RightDockWidgetArea,this);
    }
  }
}
void GhostDock::populateComboBox(bool enabled)
{
  if(enabled){
    QList<int> idxs = InputEventManager::instance()->similarEvents(*m_ghost->event());
//    QList<InputEvent> events 
//    = InputEventManager::instance()->InputEvents();
//    foreach(int idx, idxs){
//      qDebug() << events.at(idx).str().left(5);
//    }
  }
}
