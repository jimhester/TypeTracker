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
    void randomize();

  public:
    InputLesson(const InputEvent &event, QWidget* parent = 0);
    void setManager(InputEventManager* manager);

  private slots:
    void processEvent();
    void timeout();
  private:
	  void setSelectedTextColor(QColor color, QTextCursor* cursor,bool background=false);
	  void addGhost();

    ~InputLesson(void);

    QTimer* m_timer;

    int m_timeout;
    int m_location;
    InputEvent m_baseEvent;
    InputLessonTextEdit* m_lesson;

    InputEventGhost* m_ghost;
    InputEventManager* m_manager;
};

class InputLessonTextEdit : public QTextEdit
{
  Q_OBJECT

  signals:
    void eventOccured();
  
  public:
    InputLessonTextEdit(const InputEvent& event, QWidget* parent = 0);
    void setInputEvent(const InputEvent& event);
  protected:
    void keyPressEvent(QKeyEvent* event);
    void mouseReleaseEvent(QMouseEvent* e);
  private:
    void deleteChar();
    void addCorrectChar(QChar chr);
    void addIncorrectChar(QChar chr);
	  void setSelectedTextColor(QColor color, QTextCursor* cursor,bool background=false);

    int m_location;
    InputEvent m_baseEvent;
    InputEvent m_buf;
    QTime* m_time;
};

class GhostDock;
class InputEventGhost : public QTextEdit 
{

  Q_OBJECT

  public:
    InputEventGhost(const InputEvent &event, QWidget* parent = 0);
    ~InputEventGhost();

    void start();
    void pause();
    void stop();
    void rewind(int msec);
    void clear();

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
class GhostDock : public QDockWidget , private Ui::GhostDock
{
  Q_OBJECT

    //friend InputEventGhost;

  public slots:
  void populateComboBox(bool enabled);

  public:
  GhostDock(InputEventGhost* ghost, QWidget* parent = 0);

  private:

  InputEventGhost* m_ghost;
};
