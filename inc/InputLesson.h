#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QDockWidget>
#include "InputEvent.h"
#include "ui_ghostDock.h"

class QTextCursor;
class QTime;
class InputEventGhost;
class InputLessonTextEdit;
class InputLesson :
  public QWidget
{
  Q_OBJECT

  public slots:
    void reset();
    void randomize();
    void showGhost(bool);

  public:
    InputLesson(const InputEvent &event, QWidget* parent = 0);
    void setManager(InputEventManager* manager);

  private slots:
    void processEvent();
    void completeEvent();
    void timeout();
    void moveGhost(int from,int to);
    void switchGhost(int which);
  private:
	  void setupGhosts();

    ~InputLesson(void);

    QTimer* m_timer;

    int m_timeout;
    int m_location;
    InputEvent m_baseEvent;
    QList<int> m_similarEvents;
    InputLessonTextEdit* m_lesson;

    InputEventGhost* m_ghost;
    QMap<InputEventGhost*,QColor> m_ghostColors;
    InputEventManager* m_manager;
    QPushButton* m_randomizeButton;
    QPushButton* m_resetButton;
    QComboBox* m_ghostComboBox;
};

class InputLessonTextEdit : public QTextEdit
{
  Q_OBJECT

  signals:
    void eventOccured();
    void eventComplete();

  public slots:

  public:
    InputLessonTextEdit(const InputEvent& event, QWidget* parent = 0);
    void setInputEvent(const InputEvent& event);
    InputEvent inputEvent() const;
    void moveGhost(int from,int to,QColor col = Qt::blue);
  protected:
    void keyPressEvent(QKeyEvent* event);
    void mouseReleaseEvent(QMouseEvent* e);
  private:
    void deleteChar();
    void addCorrectChar(QChar chr);
    void addIncorrectChar(QChar chr);

    int m_location;
    InputEvent m_baseEvent;
    InputEvent m_buf;
    QTime* m_time;
};

class GhostDock;
class InputEventGhost : public QTextEdit 
{

  Q_OBJECT

  signals:
    void changeLocation(int, int);
  
  public slots:
    void start();
    void pause();
    void stop();
    void clear();
    void rewind(int msec=1000);

  public:
    InputEventGhost(const InputEvent &event, QWidget* parent = 0);
    ~InputEventGhost();

    bool isActive() const;
    bool isComplete() const;

    void setEvent(const InputEvent& event);
    const InputEvent* event() const;

    private slots:
      void nextKey();

  private:
    void setupDock();

    InputEvent m_event;
    QTextCursor* m_cursor;
    QTimer* m_timer;
    QVector<int> m_times;
    QString m_text;
    int m_location;
    bool m_running;
    //GhostDock* m_dock;
};