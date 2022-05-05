#pragma once

#include "zmdichild.h"

class ZPosts : public ZMdiChild
{

public:
	ZPosts(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};

