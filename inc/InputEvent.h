#pragma once

#include <QHash>
#include <QVector>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QAbstractProxyModel>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QVariant>
#include <QStringList>
#include <QString>

class QByteArray;
class QSqlQuery;

struct count{
	int number;
	int error;
	int totalTime;
};

Q_DECLARE_METATYPE(count)
typedef QHash<QString,count> CountHash;
Q_DECLARE_METATYPE(CountHash)

class InputEvent
{
public:
	InputEvent(void);
	InputEvent(const QString & keys, int time=0);
	InputEvent(const QString & keys, const QVector<int> & times = QVector<int>(), const QDateTime & date=QDateTime(), const QVector<bool> & errors=QVector<bool>());
	InputEvent(const QString & keys, const QByteArray & time=QByteArray(), const QDateTime & n_date=QDateTime());
	~InputEvent(void);
	void addKey(const QString & key, int msec,bool isCorrect=true);
	void setDate(const QDateTime &datetime);
	static void setValidator(const QRegExp &regex);
	static QRegExp validator();
	void clear();
	bool isEmpty() const;
	bool isValid() const;
	QString keys() const;
	QVector<int> times() const;
	QDateTime date() const;
	QString str() const;
	int totalTime() const;
	double trueWordsPerMinute() const;
	double normalizedWordsPerMinute(int size=5) const;
	double charactersPerMinute() const;
	double percentCorrect() const;
	const CountHash& substr(int size=1,bool allowSpace=true) const;
	const CountHash& words() const;
	bool operator==(const InputEvent& event);

private:
	void process() const;

	enum Statistics {
		TotalTime,
		TrueWordsPerMinute,
		NormalizedWordsPerMinute,
		CharactersPerMinute,
		PercentCorrect
	};

	mutable QVector<QVariant> m_statCache;

	QString m_keys;
	QVector<int> m_times;
	QVector<bool> m_error;
	QDateTime m_datetime;

	mutable bool m_processed;
	mutable QString m_final;
	mutable QVector<bool> m_finalErrors;
	mutable QVector<int> m_finalTimes;
	mutable QHash<int, CountHash> m_substr;
	mutable CountHash m_words;

	static QRegExp m_regexp;
};

class InputEventModel;
class InputEventTreeModel;
class InputEventManager;
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

    QList<int> similarEvents(const InputEvent &evnt,double difference=0.0);

    InputEventModel *inputEventModel() const;

	static InputEventManager* instance();

public slots:
    void clear();

private slots:
    void save();

private:
    void load();
	void insertInputEvent(const InputEvent &item);
	QString hashInputEvent(const InputEvent &item);

    int m_InputEventLimit;
    QTimer m_expiredTimer;
    QList<InputEvent> m_InputEvents;

    InputEventModel *m_InputEventModel;
	InputEventTreeModel *m_InputEventTreeModel;

	QHash<QString,QList<int> > m_similarity;

	InputEventManager(const InputEventManager&);
	InputEventManager& operator=(const InputEventManager &);
	static InputEventManager* m_inputEventManager;
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
		ItemOffsetRole = Qt::UserRole + 1,
		WordRole = Qt::UserRole + 2,
        SubstrRole = Qt::UserRole + 3, //the size of the substr is role #-SubstrRole+1
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
	mutable CountHash m_substrSums;
	int m_substrLength;

};

//Filters selected rows
class InputEventFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	InputEventFilterModel(QAbstractItemModel *sourceModel,QList<int> rows,QObject *parent = 0);
	bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex & source_parent) const;
private:
	QList<int> m_rows;
};