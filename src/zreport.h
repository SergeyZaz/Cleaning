#pragma once
#include <QItemDelegate>
#include <QListWidget>
#include <QTextEdit>
#include <QWidget>
#include "ui_zreport.h"

class ZReport : public QWidget
{
	Q_OBJECT

	struct tariff_history
	{
		QDate date;
		double val;
	};
	struct tariff
	{
		int id, mode, type;
		double bonus;
		QString name;
		QList< tariff_history > l_history;
	};
	QList<tariff> l_tariffs;
	int curFindId;

	double getTariffValue(const QDate& date, int id, int num, QString& txt, double& bonus, double& t_val, bool& fSmena);
	void loadItemsTo(QComboBox* cbo, const QString& tableName);
	void loadTariffs();
	QSize	sizeHint() const;
	double getSumma(QTreeWidgetItem* pItemRoot, int col);
	void updateAllSumm(const QList<int>& cols);
	void roundSumm();
	int findText(const QString& text);


public:
	ZReport(QWidget* parent, Qt::WindowFlags flags = 0);
	~ZReport();
	int getTextForPayment(int id, int col, QString& text, QVariantList &vList, double &summa, QString& comm);
	void updateSumm();
	int updateComment(int id, const QString& text);

	Ui::ZReport ui;

public slots:
	void buildProtokol();
	void saveProtokol();
	void expandAll(bool fCheck);
	void findNextSlot();
	void findFirstSlot(const QString& text);
	void changeOrgSlot();

};


class ZTreeDataDelegate : public QItemDelegate
{
	Q_OBJECT

	ZReport* pEditor;
	mutable QWidget* w;
	mutable QTextEdit* textEdit;
	mutable QListWidget* listWidget;
	mutable int column;
	mutable int fio_id;
	
	int openEditor(int id);

public:
	ZTreeDataDelegate(ZReport*Editor, QObject* parent = 0);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor,
		const QStyleOptionViewItem& option, const QModelIndex& index) const;

public slots:
	void add_clicked();
	void edit_clicked();
	void del_clicked();
};