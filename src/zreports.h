#pragma once

#include <QWidget>
#include <QItemDelegate>
#include <QItemDelegate>
#include "ui_zreport.h"
#include "zview.h"


class ZReportsModel : public QAbstractListModel
{
	static QStringList n_days;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex&) const;

public:
	ZReportsModel(QObject* parent = 0);
	~ZReportsModel();
	int rowCount(const QModelIndex&) const { return m_data.size(); }
	int columnCount(const QModelIndex&) const { return headers_before.size() + headers_after.size() + d_begin.daysTo(d_end) + 1; }
	QVariant data(const QModelIndex& index, int role) const;
	void Update();
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	ZSortFilterProxyModel* sortModel;

	QDate d_begin, d_end;
	QStringList headers_before, headers_after;

	struct elem
	{
		int post_id, fio_id, id;
		QString post, fio, org, comment;
		QMap<int, int> vars;
		double tariff, doplata, vichet, zp;
		elem()
		{
			post_id = fio_id = 0;
			tariff = doplata = vichet = zp = 0.0;
		}
	};
	QList<elem> m_data;

	enum op_type {UP_COMMENT, UP_VALUE, DEL_VALUE};
	bool updateDataDB(op_type t, elem* pData, int d = -1);
};

class ZReports : public QWidget
{
	Q_OBJECT

	Ui::ZReport ui;
	int curFindId;

	ZReportsModel model;

	void buildReport();
	int findText(const QString& text);
	QSize sizeHint() const;

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
	static QMap<int, QString> map;

	ZReportsDelegate(QObject* parent = 0);

	static void load();

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor,
		const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
