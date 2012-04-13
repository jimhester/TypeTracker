#include "InputLesson.h"
#include <QBoxLayout>
#include <QTextEdit>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTime>
#include <QDockWidget>
#include <QGridLayout>
#include <QMainWindow>
#include <QGroupBox>
#include "ttEdit.h"
#include "colorwizard.h"

qreal pointSize = 20;
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
  QHBoxLayout* layout = new QHBoxLayout;
  QVBoxLayout* inputLayout = new QVBoxLayout;
  QGroupBox* groupBox = new QGroupBox("&Ghost Options");
  groupBox->setCheckable(true);
  QVBoxLayout* controlLayout = new QVBoxLayout;

  m_lesson = new InputLessonTextEdit(m_baseEvent,this);

  inputLayout->setDirection(QVBoxLayout::BottomToTop);
  
  inputLayout->addWidget(m_lesson);

  m_ghost = new InputEventGhost(m_baseEvent,this);
  inputLayout->addWidget(m_ghost);
  m_randomizeButton = new QPushButton("&Randomize", this);
  connect(m_randomizeButton, SIGNAL(clicked() ),this, SLOT(randomize() ) );
  m_resetButton = new QPushButton("&Reset", this);
  QCheckBox* check = new QCheckBox("&Show Ghost",this);
  connect(check, SIGNAL(toggled(bool) ), this, SLOT(showGhost(bool)));
  m_ghostComboBox = new QComboBox();
  controlLayout->addWidget(check);
  controlLayout->addWidget(m_ghostComboBox);
  controlLayout->addWidget(m_randomizeButton);
  controlLayout->addWidget(m_resetButton);
  controlLayout->addStretch();
  layout->addLayout(inputLayout);
  layout->addLayout(controlLayout);
  setLayout(layout);

  m_timer = new QTimer;
  m_timer->setSingleShot(true);

  connect(m_resetButton, SIGNAL(clicked()), this, SLOT(reset()));
  connect(m_lesson, SIGNAL(eventOccured() ), this, SLOT( processEvent() ) );
  connect(m_lesson, SIGNAL(eventComplete() ), this, SLOT( completeEvent() ) );
  connect(m_ghostComboBox, SIGNAL(activated(int) ), this, SLOT(switchGhost(int)));
}
InputLesson::~InputLesson(void)
{
}
void InputLesson::showGhost(bool show)
{
  if(show){
    m_ghost->show();
  } else {
    m_ghost->hide();
  }
}
void InputLesson::moveGhost(int from,int to)
{
  InputEventGhost* ghost = static_cast<InputEventGhost*>(QObject::sender());
  m_lesson->moveGhost(from,to,m_ghostColors.value(ghost));
}
void InputLesson::processEvent()
{
  if(!m_timer->isActive()){
    foreach(InputEventGhost* ghost,m_ghostColors.keys()){
      ghost->start();
    }
    m_ghost->start();
  }
  m_timer->start(m_timeout);
}
void InputLesson::completeEvent()
{
  InputEvent evt = m_lesson->inputEvent();
  if(evt.isValid()){
    evt.setDate(QDateTime::currentDateTime());
    m_manager->addInputEvent(evt);
    //TODO display stats about event
  }
}
void InputLesson::setManager(InputEventManager* manager)
{
  m_manager = manager;
  setupGhosts();
}
void InputLesson::setupGhosts()
{
  if(m_manager){
    m_ghostColors.clear();
    m_similarEvents = m_manager->similarEvents(m_baseEvent);
    QVector<QColor> cols = ColorWizard::highlight(Qt::white,Qt::black,3);
    QList<InputEventGhost*> ghosts;
    ghosts.append(m_ghost);
    int currentEventIdx = 0;
    if(m_similarEvents.size() > 2){
      QPair<int,double> min(-1,10000.0);
      QPair<int,double> max(-1,-1);
      int itr=0;
      foreach(int idx, m_similarEvents){
        InputEvent evnt = m_manager->InputEvents().at(idx);
        if(evnt == m_baseEvent){
          currentEventIdx = itr;
        }
        double speed = evnt.normalizedWordsPerMinute();
        if(speed < min.second){
          min.first = idx;
          min.second = speed;
        }
        if(speed > max.second){
          max.first = idx;
          max.second = speed;
        }
        itr++;
      }
      ghosts.append(new InputEventGhost(m_manager->InputEvents().at(min.first).mapToPermutation(m_baseEvent),this));
      ghosts.append(new InputEventGhost(m_manager->InputEvents().at(max.first).mapToPermutation(m_baseEvent),this));
    }
    InputEventFilterModel* filter = new InputEventFilterModel(m_manager->inputEventModel(),m_similarEvents,this);
    m_ghostComboBox->setModel(filter);
    m_ghostComboBox->setModelColumn(4);
    m_ghostComboBox->setCurrentIndex(currentEventIdx);
    int itr=0;
    foreach(InputEventGhost* ghost, ghosts){
      ghost->hide();
      m_ghostColors[ghost]=cols[itr];
      connect(ghost, SIGNAL(changeLocation(int, int) ), this, SLOT( moveGhost(int, int) ) );
      connect(m_timer, SIGNAL(timeout()), ghost, SLOT(pause()));
      connect(m_randomizeButton, SIGNAL(clicked() ),ghost, SLOT(clear()));
      connect(m_resetButton, SIGNAL(clicked() ),ghost, SLOT(clear()));
      connect(m_lesson, SIGNAL(eventCompleted() ), ghost, SLOT(stop()));
      itr++;
    }
  }
}
void InputLesson::switchGhost(int which)
{
  m_ghost->setEvent(m_manager->InputEvents().at(m_similarEvents.at(which)).mapToPermutation(m_baseEvent));
}
void InputLesson::timeout()
{
  m_ghost->pause();
  m_ghost->rewind(m_timeout);
}
void InputLesson::randomize()
{
  m_baseEvent = m_baseEvent.randomizeEvent();
  m_lesson->setInputEvent(m_baseEvent);
  m_ghost->clear();
  m_ghost->setEvent(m_baseEvent);
}
void InputLesson::reset()
{
  m_lesson->setInputEvent(m_baseEvent);
  m_ghost->clear();
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
  setTextBackgroundColor(Qt::white);
  setTextInteractionFlags(Qt::TextEditable);
  setText(m_baseEvent.str());
}
InputEvent InputLessonTextEdit::inputEvent() const
{
  return m_buf;
}
  
void InputLessonTextEdit::keyPressEvent(QKeyEvent* event)
{
  if( !(textInteractionFlags() & Qt::TextEditable) ) return;
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
  if(m_location >= m_baseEvent.str().size()){
    emit eventComplete();
    setTextInteractionFlags(Qt::NoTextInteraction);
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
    if(chr.isSpace()){
      fmt.setBackground(fmt.foreground());
    }
    cursor.mergeCharFormat(fmt);
    cursor.insertText(chr);
    m_location++;
}
void InputLessonTextEdit::moveGhost(int from,int to,QColor col)
{
  QTextCursor cursor = textCursor();
  QTextCharFormat fmt;
  cursor.setPosition(from);
  cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
  fmt.setFontUnderline(false);
  cursor.mergeCharFormat(fmt);
  
  fmt.setUnderlineColor(col);
  fmt.setFontUnderline(true);
  cursor.setPosition(to);
  cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
  cursor.mergeCharFormat(fmt);
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
    emit changeLocation(m_cursor->position(),m_cursor->position()-1);
  } else {
    m_cursor->insertText(m_text.at(m_location));
    emit changeLocation(m_cursor->position()-2,m_cursor->position()-1);
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
  pause();
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