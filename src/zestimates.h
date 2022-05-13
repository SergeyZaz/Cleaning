#pragma once

#include "zmdichild.h"
#include "ui_zestimatesform.h"
#include "zeditbaseform.h"

#include <QTreeWidget>
#include <QItemDelegate>
#include <QComboBox>
#include <QSpinBox>
#include <QMap>

class ZEstimatesForm : public ZEditAbstractForm
{
	Q_OBJECT
		
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