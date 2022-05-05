#pragma once

#include "zmdichild.h"


class ZPeriods : public ZMdiChild
{

public:
	ZPeriods(QWidget* parent, Qt::WindowFlags flags = 0);
 
	void init(const QString &m_TblName);
};


