#pragma once

#include "zmdichild.h"
#include "ui_zestimatesform.h"
#include "zeditbaseform.h"

class ZEstimatesForm : public ZEditAbstractForm
{
	Q_OBJECT

public:
	ZEstimatesForm(QWidget* parent, Qt::WindowFlags flags = 0);
	~ZEstimatesForm();

	int init(const QString& table, int id);
	int copyData(int curId, int newId);
	Ui::ZEstimatesForm ui;

protected slots:
	void applyChanges();
	void addNewSlot();
};

class ZEstimates : public ZMdiChild
{

public:
	ZEstimates(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};
