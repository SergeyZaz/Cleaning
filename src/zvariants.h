#pragma once

#include "zmdichild.h"

class ZVariants : public ZMdiChild
{

public:
	ZVariants(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};
