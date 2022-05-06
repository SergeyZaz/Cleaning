#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include "ui_zmainwindow.h"

class ZMainWindow : public QMainWindow
{
	Q_OBJECT

	QSqlDatabase	db;
	Ui::ZMainWindow	ui;

public:
	ZMainWindow();
	~ZMainWindow();

protected:
	void	closeEvent(QCloseEvent *event);
	int		readIniFile();

private slots:
	void slotAbout();
	void slotOpenObjectsDialog();
	void slotOpenPeriodsDialog();
	void slotOpenOrganisationsDialog();
	void slotOpenPostsDialog();
	void slotOpenTariffsDialog();
	void slotOpenPersonsDialog();
	void slotOpenPaymentsDialog();
	void slotOpenReportsDialog();
	void slotOpenPaymentsFioDialog();
	void slotOpenDeductionsFioDialog();

	void slotUpdate();
	void slotCleanMsg();
	void slotSaveMsg();
	void slotOpenUsersDialog();
	void slotOpenConfigDialog();

private:
	void readSettings();
	void writeSettings();

};
