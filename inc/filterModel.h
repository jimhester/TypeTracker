#pragma once
#include <QtGui>
#include <QxtSpanSlider>

struct range{
  qreal low;
  qreal high;
};

//Displays a slider with values, and allows 2 way filtering of the values
class FilterModel;
class SpanSlider;
class FilterModelWidget : public QWidget
{
  Q_OBJECT

public slots:
  void rangeChanged();
  void newFilter();

public:
  FilterModelWidget(QAbstractItemModel* sourceModel, QWidget* parent=0);
  QAbstractItemModel* model() const;

private:
  FilterModel* m_filterModel;
  QStringList m_colnames;
  QMap<int,SpanSlider*> m_filtSpanSliders;
  QComboBox* m_combo;
};

class FilterModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  FilterModel(QAbstractItemModel* sourceModel, QWidget* parent=0);
  void setRange(int col,range r);
  QMap<int,range> ranges() const;

protected:
  bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;
  bool rowAccepted(int source_row,const QModelIndex & source_parent ) const;
  bool hasAcceptedChildren(int source_row,const QModelIndex & source_parent ) const;
private:
  QAbstractItemModel* m_sourceModel;
  QMap<int,range> m_ranges;
};

class SpanSlider : public QxtSpanSlider
{
  Q_OBJECT

signals:
  void lowerPositionChanged(double);
  void upperPositionChanged(double);

public slots:
    void setMinimum(double value);
    void setMaximum(double value);
    void setLowerPosition(double lower);
    void setUpperPosition(double upper);

public:
    explicit SpanSlider(QWidget* parent = 0);
    explicit SpanSlider(Qt::Orientation orientation, QWidget* parent = 0);

    double lowerValue() const;
    double upperValue() const;

private:
  double m_maximum;
  double m_minimum;

private slots:
  void emitDoubleLowerPositionChanged(int);
  void emitDoubleUpperPositionChanged(int);
};