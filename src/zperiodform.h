#pragma once

#include "ui_zperiodform.h"
#include "zeditbaseform.h"

class ZPeriodForm : public ZEditAbstractForm
{
    Q_OBJECT

public:
	ZPeriodForm(QWidget* parent, Qt::WindowFlags flags = 0);
	~ZPeriodForm();

	int init(const QString& table, int id);
	Ui::ZPeriodForm ui;

protected slots:
	void applyChanges();
	void addNewSlot();
};

