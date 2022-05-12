#pragma once

#include "zmdichild.h"
#include "ui_zcontractsform.h"
#include "zeditbaseform.h"

class ZContractsForm : public ZEditAbstractForm
{
	Q_OBJECT

public:
	ZContractsForm(QWidget* parent, Qt::WindowFlags flags = 0);
	~ZContractsForm();

	int init(const QString& table, int id);
	Ui::ZContractsForm ui;

protected slots:
	void applyChanges();
	void addNewSlot();
};
class ZContracts : public ZMdiChild
{

public:
	ZContracts(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};
