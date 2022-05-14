#pragma once

#include "zmdichild.h"
#include "ui_zestimatesform.h"
#include "zeditbaseform.h"

#include <QAbstractListModel>
#include <QTreeWidget>
#include <QItemDelegate>
#include <QComboBox>
#include <QSpinBox>
#include <QMap>


class ZEstimatesFioModel : public QAbstractListModel
{
	QStringList headers;
	uint		rows;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex&) const;

public:
	ZEstimatesFioModel(QObject* parent = 0);
	~ZEstimatesFioModel();
	void setRowCount(int n);
	int rowCount(const QModelIndex& parent) const { return rows; }
	int columnCount(const QModelIndex& parent) const { return headers.size(); }
	QVariant data(const QModelIndex& index, int role) const;
	void Update();
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	ZSortFilterProxyModel* sortModel;

	struct elem
	{
		int id1, id2;
		QString txt1, txt2, comment;
	};
	QMap<int, elem> m_data;
};

class ZEstimatesForm : public ZEditAbstractForm
{
	Q_OBJECT
		
	ZEstimatesFioModel	postsModel, worksModel;
	int loadTblInfo(const QString& tbl, QTreeWidget *tree);

public:
	ZEstimatesForm(QWidget* parent, Qt::WindowFlags flags = 0);
	~ZEstimatesForm();

	int init(const QString& table, int id);
	int copyData(int curId, int newId);
	void updateSumm();

	Ui::ZEstimatesForm ui;

	struct linkInfo
	{
		int id;
		QString name;
		int parent_id;
	};
	QMap<int, linkInfo> mapPosts, mapWorks;

protected slots:
	void applyChanges();
	void addNewSlot();
	void addRowSlot();
	void delRowSlot();
	void itemChangedSlot(QTreeWidgetItem*, int);
	void dateChangedSlot(const QDate&);
	void changePeriodSlot(int);
};

class ZEstimates : public ZMdiChild
{

public:
	ZEstimates(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};


class ZEstimatesTreeDelegate : public QItemDelegate
{
	QString mTblName;
	ZEstimatesForm* pParent;
public:
	ZEstimatesTreeDelegate(ZEstimatesForm *p, const QString& tbl, QObject* parent = 0);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor,
		const QStyleOptionViewItem& option, const QModelIndex& index) const;
};


class ZEstimatesFioDelegate : public QItemDelegate
{
public:
	ZEstimatesFioDelegate(QObject* parent = 0);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor,
		const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
