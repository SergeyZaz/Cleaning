#pragma once

#include "zmdichild.h"
#include "ui_zpostsform.h"
#include "zeditbaseform.h"

class ZPostsForm : public ZEditAbstractForm
{
	Q_OBJECT

public:
	ZPostsForm(QWidget* parent, Qt::WindowFlags flags = 0);
	~ZPostsForm();

	int init(const QString& table, int id);
	Ui::ZPostsForm ui;

protected slots:
	void applyChanges();
	void addNewSlot();
};


class ZPosts : public ZMdiChild
{

public:
	ZPosts(QWidget* parent, Qt::WindowFlags flags = 0);

	void init(const QString& m_TblName);
};

