#pragma once
#include <QtGui>
#include <QAbstractItemModel>
#include <QIdentityProxyModel>
#include <QWidget>
#include "InputEvent.h"

class SubstringModel;
class SubstringAnalysis : public QWidget
{
  Q_OBJECT

  public slots:
    void lengthChanged(int value);
    void spacesChanged(bool value);

  public:
    SubstringAnalysis(QAbstractItemModel* sourceModel, QWidget* parent=0);
    ~SubstringAnalysis(void);
  private:
    QList<int> m_rows;
    QAbstractItemView* m_view;
    InputEventFilterModel * m_filter;
    InputEventTreeModel* m_tree;
    SubstringModel* m_substr;
};

class SubstringModel : public QIdentityProxyModel
{
  Q_OBJECT

  public slots:
      void setAllowSpaces(bool value);
      void setSubstringLength(int length);
  public:
    SubstringModel(QAbstractItemModel* sourceModel,QObject* parent);
    QVariant data(const QModelIndex &index, int role) const;

  private:
    bool m_allowSpaces;
    int m_substrLength;
};