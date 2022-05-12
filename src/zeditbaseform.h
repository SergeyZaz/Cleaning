#pragma once
#include <QDialog>
#include <QDateTime>
#include <QComboBox>
#include <QSqlQuery>
#include "ui_zeditbaseform.h"

#define ADD_UNIC_CODE	-99

class ZEditAbstractForm : public QDialog
{
	Q_OBJECT
public:
	int						oldId;
	int						curEditId;
	QString					m_tbl;

	ZEditAbstractForm(QWidget *parent = 0, Qt::WindowFlags flags = 0):QDialog(parent, flags)
	{
		oldId = ADD_UNIC_CODE;
		setModal(true);
	}
	virtual int init(const QString &tbl, int id)
	{
		m_tbl = tbl;
		curEditId = id;
		return 1;
	}
	virtual ~ZEditAbstractForm(){};
	virtual void setSectionsType(int) {};
	virtual int copyData(int curId, int newId) { return 1; }

signals:	
	void errorQuery(const QDateTime &, long , const QString &);
	void needUpdateVal(int);
};

class ZEditBaseForm : public ZEditAbstractForm
{
	Q_OBJECT
	Ui::EditBaseForm ui;
	int fNeedComment;
	void closeEvent(QCloseEvent *event);
	int applyChange();
public:
	int init(const QString &tbl, int id);
	ZEditBaseForm(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~ZEditBaseForm();
public slots:
	void applySlot();
	void addNewSlot();
	void textChangedSlot(const QString &);
};
