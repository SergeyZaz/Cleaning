#pragma once

#include "zmdichild.h"

class ZWorks : public ZMdiChild
{

public:
	ZWorks(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};
