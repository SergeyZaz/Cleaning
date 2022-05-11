#pragma once

#include "ui_zpayments2fioform.h"
#include "zeditbaseform.h"

class ZPayments2FioForm : public ZEditAbstractForm
{
    Q_OBJECT

	int mMode; // 0-выплата, 1-вычет
	void loadFio();

public:
	ZPayments2FioForm(QWidget* parent, int mode, Qt::WindowFlags flags = 0);
	~ZPayments2FioForm();

	int init(const QString& table, int id);
	Ui::ZPayments2FioForm ui;

protected slots:
	void applyChanges();
	void addNewSlot();
};

