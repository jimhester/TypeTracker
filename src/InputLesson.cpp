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
  QVBoxLayout* layout = new QVBoxLayout;
  m_lesson = new InputLessonTextEdit(m_baseEvent,this);

  layout->setDirection(QVBoxLayout::BottomToTop);
  
  layout->addWidget(m_lesson);

  m_ghost = new InputEventGhost(m_baseEvent,this);
  layout->addWidget(m_ghost);

  QPushButton *button = new QPushButton("&Randomize", this);
  connect(button, SIGNAL(clicked() ),this, SLOT(randomize() ) );
  layout->addWidget(button);

  setLayout(layout);

  m_timer = new QTimer;
  m_timer->setSingleShot(true);

  connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
  connect(m_lesson, SIGNAL(eventOccured() ), this, SLOT( processEvent() ) );
}
InputLesson::~InputLesson(void)
{
  if(m_ghost)
    m_ghost->close();
}
void InputLesson::processEvent()
{
  if(!m_timer->isActive()){
    m_ghost->start();
  }
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

void InputLesson::timeout()
{
  m_ghost->pause();
  m_ghost->rewind(m_timeout);
  m_timer->stop();
}
void InputLesson::randomize()
{
  InputEvent newEvent = m_baseEvent.randomizeEvent();
  m_baseEvent = newEvent;
  m_lesson->setInputEvent(m_baseEvent);
  m_ghost->clear();
  m_ghost->setEvent(m_baseEvent);
}
InputLessonTextEdit::InputLessonTextEdit(const InputEvent& event, QWidget* parent)
  : QTextEdit(parent) ,
  m_baseEvent(event) ,
  m_location(0) ,
  m_time(0)
{
  m_time = new QTime();
  setFontPointSize(pointSize);
  setTextColor(Qt::gray);
  setText(m_baseEvent.str());

}
void InputLessonTextEdit::setInputEvent(const InputEvent& event)
{
  m_baseEvent = event;
  m_location = 0;
  m_buf.clear();
  setFontPointSize(pointSize);
  setTextColor(Qt::gray);
  setTextInteractionFlags(Qt::TextEditable);
  setText(m_baseEvent.str());
}
  
void InputLessonTextEdit::keyPressEvent(QKeyEvent* event)
{
  if(textInteractionFlags() & Qt::TextEditable){
      int msec = m_time->restart();
      if(msec > 1000)
        msec = 0;
      if(!event->text().isEmpty()){
        if(event->text() == QChar(0x08)){
          m_buf.addKey(event->text(),msec,false);
          deleteChar();
        } else {
          bool correct = event->text() == m_baseEvent.str().at(m_location);
          m_buf.addKey(event->text(),msec,correct);
          if(correct){
            addCorrectChar(event->text().at(0));
          } else {
            addIncorrectChar(event->text().at(0));
          }
        }
      }
      emit eventOccured();
  }
}
void InputLessonTextEdit::mouseReleaseEvent(QMouseEvent* event)
{
  QTextEdit::mouseReleaseEvent(event);
}

void InputLessonTextEdit::deleteChar()
{
  if(m_location > 0){
    QTextCursor cursor = textCursor();
    QTextCharFormat fmt = cursor.charFormat();
    fmt.setBackground(Qt::white);
    fmt.setForeground(Qt::gray);
    cursor.setPosition(m_location);
    cursor.setPosition(m_location-1,QTextCursor::KeepAnchor);
    cursor.insertText(m_baseEvent.str().mid(m_location-1,1),fmt);
    cursor.setPosition(m_location-1);
    setTextCursor(cursor);
    m_location--;
  }
}
void InputLessonTextEdit::addCorrectChar(QChar chr)
{
  if(m_location == m_baseEvent.str().size()-1 && m_baseEvent.isValid()){
    m_buf.setDate(QDateTime::currentDateTime());
    InputEventManager::instance()->addInputEvent(m_buf);
    setTextInteractionFlags(Qt::NoTextInteraction);
  }
  QTextCursor cursor = textCursor();
  QTextCharFormat fmt;
  fmt.setBackground(Qt::white);
  fmt.setForeground(Qt::black);
  cursor.setPosition(m_location);
  cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
  cursor.mergeCharFormat(fmt);
  cursor.insertText(chr);
  m_location++;
}
void InputLessonTextEdit::addIncorrectChar(QChar chr)
{
    QTextCursor cursor = textCursor();
    QTextCharFormat fmt;
    fmt.setBackground(Qt::white);
    fmt.setForeground(Qt::red);

    cursor.setPosition(m_location);
    cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
    if(cursor.selectedText().at(0).isSpace()){
      fmt.setBackground(fmt.foreground());
    }
    cursor.mergeCharFormat(fmt);
    cursor.insertText(chr);
    m_location++;
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

}
InputEventGhost::~InputEventGhost()
{
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
void InputEventGhost::clear()
{
  setText("");
  QTextCharFormat fmt;
  fmt.setFontPointSize(pointSize);
  m_cursor->mergeCharFormat(fmt);
  stop();
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
