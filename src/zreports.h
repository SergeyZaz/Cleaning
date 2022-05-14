#pragma once

#include <QWidget>
#include <QItemDelegate>
#include <QItemDelegate>
#include "ui_zreport.h"
#include "zview.h"


class ZReportsModel : public QAbstractListModel
{
	QStringList headers_before, headers_after, n_days;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex&) const;

public:
	ZReportsModel(QObject* parent = 0);
	~ZReportsModel();
	int rowCount(const QModelIndex&) const { return m_data.size(); }
	int columnCount(const QModelIndex&) const { return headers_before.size() + headers_after.size() + d_begin.daysTo(d_end); }
	QVariant data(const QModelIndex& index, int role) const;
	void Update();
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	ZSortFilterProxyModel* sortModel;

	QDate d_begin, d_end;

	struct elem
	{
		int post_id, fio_id;
		QString post, fio;
		QMap<int, int> vars;
		double tariff, summ;
	};
	QList<elem> m_data;
};

class ZReports : public QWidget
{
	Ui::ZReport ui;
	int curFindId;

	ZReportsModel model;

	void buildReport();
	int findText(const QString& text);

public:
	ZReports(QWidget* parent, Qt::WindowFlags flags = 0);
	~ZReports();
	

public slots:
	void changeEstimateSlot(int);
	void changePeriodSlot(int);
	void findNextSlot();
	void findFirstSlot(const QString& text);
};


class ZReportsDelegate : public QItemDelegate
{
public:
	ZReportsDelegate(QObject* parent = 0);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor,
		const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
