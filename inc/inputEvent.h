#pragma once

#include <QHash>
#include <QVector>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QAbstractProxyModel>
#include <QTimer>
#include <qVariant>
#include <QStringList>

class QByteArray;
class QSqlQuery;
class QString;

struct count{
	int number;
	int error;
	int totalTime;
};

Q_DECLARE_METATYPE(count)
Q_DECLARE_TYPEINFO(count,Q_PRIMITIVE_TYPE);
//int id = qRegisterMetaType<count>();

class InputEvent
{
public:
	InputEvent(void);
	InputEvent(const QByteArray & str, int time=0);
	InputEvent(const QByteArray & str, const QByteArray & time);
	InputEvent(const QByteArray & str, const QByteArray & time, const QDateTime & n_date);
	~InputEvent(void);
	void addKey(const QString & key, int msec);
	void clear();
	bool isEmpty() const;
	bool isValid(); //const;
	QByteArray keys() const;
	QVector<int> times() const;
	QDateTime date() const;
	QString str() const;
	int totalTime() const;
	void setDate(const QDateTime &datetime);
	const QHash<QString, count> & substr(int size=1,bool allowSpace=true);
	const QHash<QString, count> & words();

private:
	void process();

	bool m_processed;
	QByteArray m_keys;
	QVector<int> m_times;
	QDateTime m_datetime;
	QString m_final;
	QVector<bool> m_finalErrors;
	QVector<int> m_finalTimes;
	QHash<int, QHash<QString,count> > m_substr;
	QHash<QString,count> m_words;
};

class InputEventModel;
class InputEventTreeModel;
class InputEventManager : public QObject
{
    Q_OBJECT

signals:
    void InputEventReset();
    void entryAdded(const InputEvent &item);
    void entryRemoved(const InputEvent &item);
    //void entryUpdated(int offset);

public:
    InputEventManager(QObject *parent = 0);
    ~InputEventManager();

    void addInputEvent(const InputEvent &item);

    //void updateInputEvent(const QUrl &url, const QString &title);

    QList<InputEvent> InputEvents() const;
    void setInputEvents(const QList<InputEvent> &InputEvents);

	//TODO
    InputEventModel *inputEventModel() const;
    //InputEventTreeModel *inputEventTreeModel() const;

public slots:
    void clear();

private slots:
    void save();

private:
    void load();

    int m_InputEventLimit;
    QTimer m_expiredTimer;
    QList<InputEvent> m_InputEvents;

    InputEventModel *m_InputEventModel;
	InputEventTreeModel *m_InputEventTreeModel;
};

class InputEventModel : public QAbstractTableModel
{
    Q_OBJECT

public slots:
    void InputEventReset();
    void entryAdded();
    void entryUpdated(int offset);

public:
    enum Roles {
		WordRole = Qt::UserRole + 1,
        SubstrRole = Qt::UserRole + 2, //the size of the substr is role #-SubstrRole+1
    };

    InputEventModel(InputEventManager *InputEvent, QObject *parent = 0);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    InputEventManager *m_InputEvent;
};

// proxy model for the InputEvent model that converts the list
// into a tree, one top level node per substr/word
class InputEventTreeModel : public QAbstractProxyModel
{
    Q_OBJECT

public slots:
	void setSubstrLength(double substrLength);
	void setSubstrLength(int substrLength);

public:
    InputEventTreeModel(QAbstractItemModel *sourceModel, QObject *parent = 0, int substrLength = -1);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index= QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setSourceModel(QAbstractItemModel *sourceModel);

private slots:
    void sourceReset();
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);

private:
//    int sourceDateRow(int row) const;
    mutable QList< QList<int> > m_sourceRowMap;
	mutable QStringList m_substr;
	int m_substrLength;

};