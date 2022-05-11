#pragma once

#include "zmdichild.h"
#include "ui_zpayments2fio.h"

class ZPayments2fioBase : public QWidget
{
	Q_OBJECT

	int filterOrganisationId;
	int filterPaymentId;
	int filterPeriodId;
	int mMode; // 0-выплата, 1-вычет
	QDate filterDate;
	Ui::ZPayments2fioDialog ui;

public:
	ZPayments2fioBase(QWidget* parent, int mode = 0, Qt::WindowFlags flags = 0);
	void init(const QString& m_TblName);

public slots:
	void UpdateSumma(int v = -1);
	void ChangeFilter();
	void dateChangedSlot(const QDate&);
	void ImportSlot();
	void SelectionChanged(const QItemSelection&, const QItemSelection&);
	QSize	sizeHint() const { return QSize(1000, 600); }
};

class ZDeductions2fio : public ZPayments2fioBase
{
public:
	ZDeductions2fio(QWidget* parent, Qt::WindowFlags flags = 0) : ZPayments2fioBase(parent, 1, flags) {};
};


class ZPayments2fio : public ZPayments2fioBase
{
public:
	ZPayments2fio(QWidget* parent, Qt::WindowFlags flags = 0) : ZPayments2fioBase(parent, 0, flags) {};
};