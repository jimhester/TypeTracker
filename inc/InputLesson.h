#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QDockWidget>
#include "InputEvent.h"
#include "ui_ghostDock.h"

class QTextCursor;
class QTime;
class InputEventGhost;
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
    void changeCursorPosition();
    void timeout();
  private:
	  void setSelectedTextColor(QColor color, QTextCursor* cursor,bool background=false);
	  void addGhost();

    ~InputLesson(void);

    QTime* m_time;
    QTimer* m_timer;

    int m_timeout;
    int m_location;
    //QTextCursor* m_cursor;
    InputEvent m_baseEvent;
    InputEvent m_inputEvent;
    QTextEdit* m_lesson;

    InputEventGhost* m_ghost;
    InputEventManager* m_manager;
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
