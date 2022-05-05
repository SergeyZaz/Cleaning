#pragma once

#include "zmdichild.h"

class ZPayments : public ZMdiChild
{

public:
	ZPayments(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};

class ZDeductions : public ZPayments
{

public:
	ZDeductions(QWidget* parent, Qt::WindowFlags flags = 0) : ZPayments(parent, flags)
	{
	}
};
