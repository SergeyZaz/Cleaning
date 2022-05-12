#include <QMessageBox>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QInputDialog>

#include "zmainwindow.h"

#include "zobjects.h"
#include "zperiods.h"
#include "zorganisations.h"
#include "zposts.h"
#include "zpersons.h"
#include "zpayments.h"
#include "zpayments2fio.h"
#include "zsettings.h"
#include "zmessager.h"
#include "zusers.h"
#include "zauth.h"
#include "zconfigform.h"
#include "zcontracts.h"
#include "zworks.h"
#include "zcontracts.h"
#include "zvariants.h"

#include "zestimates.h"
#include "zreports.h"

#define	CFG_FILE		"cleaning.ini"
#define	PROGRAMM_NAME	"Сleaning"
#define	VERSION			"0.0.1"

ZMainWindow::ZMainWindow()
{
	ui.setupUi(this);
	setWindowTitle(PROGRAMM_NAME);

	ZMessager::Instance().setWidget(ui.msgList);

	connect(ui.actAbout, SIGNAL(triggered()), this, SLOT(slotAbout()));
	connect(ui.actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	connect(ui.actPersons, SIGNAL(triggered()), this, SLOT(slotOpenPersonsDialog()));
	connect(ui.actContracts, SIGNAL(triggered()), this, SLOT(slotOpenactContractsDialog()));
	connect(ui.actPeriods, SIGNAL(triggered()), this, SLOT(slotOpenPeriodsDialog()));
	connect(ui.actPosts, SIGNAL(triggered()), this, SLOT(slotOpenPostsDialog()));
	connect(ui.actObjects, SIGNAL(triggered()), this,	SLOT(slotOpenObjectsDialog()));
	connect(ui.actPayments, SIGNAL(triggered()), this, SLOT(slotOpenPaymentsDialog()));
	connect(ui.actWorks, SIGNAL(triggered()), this, SLOT(slotOpenWorksDialog()));
	connect(ui.actVariants, SIGNAL(triggered()), this, SLOT(slotOpenVariantsDialog()));
	connect(ui.actOrganisations, SIGNAL(triggered()), this, SLOT(slotOpenOrganisationsDialog()));

	connect(ui.actPaymentsFio, SIGNAL(triggered()), this, SLOT(slotOpenPaymentsFioDialog()));
	connect(ui.actDeductionsFio, SIGNAL(triggered()), this, SLOT(slotOpenDeductionsFioDialog()));

	connect(ui.actReports, SIGNAL(triggered()), this, SLOT(slotOpenReportsDialog()));
	connect(ui.actEstimates, SIGNAL(triggered()), this, SLOT(slotOpenEstimatesDialog()));

	connect(ui.cmdCleanMsg, SIGNAL(clicked()), this, SLOT(slotCleanMsg()));
	connect(ui.cmdSaveMsg, SIGNAL(clicked()), this, SLOT(slotSaveMsg()));
	connect(ui.actUsers, SIGNAL(triggered()), this, SLOT(slotOpenUsersDialog()));
	connect(ui.actConfig, SIGNAL(triggered()), this, SLOT(slotOpenConfigDialog()));


	readSettings();

	if(readIniFile()==0)
		exit(0);
}

ZMainWindow::~ZMainWindow()
{
}

void ZMainWindow::closeEvent(QCloseEvent *event)
{
	ui.mdiArea->closeAllSubWindows();
	if (ui.mdiArea->currentSubWindow()) 
	{
		event->ignore();
	} 
	else 
	{
		writeSettings();
		event->accept();
	}
}

void ZMainWindow::slotAbout()
{
	QMessageBox::about(this, tr("О программе"),
		QString("Программа: \"%1\".<p>Версия %4 (Сборка: %2 %3) Автор: <a href=\"mailto:zaz@29.ru\">Zaz</a>")
		.arg( windowTitle() ).arg( __DATE__ ).arg( __TIME__ ).arg(VERSION));
}


void ZMainWindow::readSettings()
{
	QSettings settings("Zaz", "DSV");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(640, 480)).toSize();

	ZSettings::Instance().m_UserName = settings.value("user", "").toString();
/*
	qint64 d = settings.value("id", 0).toLongLong();
	if (d == 0)
	{
		d = QDate::currentDate().toJulianDay();
		settings.setValue("id", d);
	}
	if (d != 159753)
	{
		d = 30 - QDate::currentDate().toJulianDay() + d;
		if (d < 0)
		{
			QMessageBox::warning(this, "Внимание", "Ознакомительный период закончился, программа будет закрыта!");
			exit(0);
		}

//		QMessageBox::warning(this, tr("Внимание"),
//			QString("Вы используете ознакомительную версию, программа перестанет работать через %1 дней!").arg(d));
	}
*/
	move(pos);
	resize(size);
}

void ZMainWindow::writeSettings()
{
	QSettings settings("Zaz", "DSV");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
	settings.setValue("user", ZSettings::Instance().m_UserName);
}

int ZMainWindow::readIniFile()
{
	if (!QFile::exists(CFG_FILE)) 
    {
		ZMessager::Instance().Message(_CriticalError, tr("Отсутствует конфигурационный файл: %1").arg(QDir::currentPath() + "/" + CFG_FILE), tr("Ошибка"));
        return 0;
    }

    QSettings settings(CFG_FILE, QSettings::IniFormat);
	settings.setIniCodec("UTF-8");

	QString str = settings.value("title").toString();
	if(!str.isEmpty())
		setWindowTitle(str);

    db = QSqlDatabase::addDatabase(settings.value("Database/driver").toString());
    db.setDatabaseName(settings.value("Database/dbname").toString());
    db.setUserName(settings.value("Database/user").toString());
    db.setPassword(settings.value("Database/pwd").toString());
    db.setHostName(settings.value("Database/host").toString());
    db.setPort(settings.value("Database/port").toInt());

    db.setConnectOptions("connect_timeout=10"); 
    if (!db.open()) 
    {
		ZMessager::Instance().Message(_CriticalError, db.lastError().text(), tr("Ошибка"));
        return 0;
    }

	ZSettings::Instance().m_Password = settings.value("password").toString();
	QString user = settings.value("user").toString();
	if (!user.isEmpty())
		ZSettings::Instance().m_UserName = user;

	ZAuthForm auth(this);
	if (auth.execute() != 1)
		return 0;

	setWindowTitle(windowTitle() + " (БД: '" + db.databaseName() + "' на '" + db.hostName() + "') пользователь: '" + ZSettings::Instance().m_UserName + "'");

	switch (ZSettings::Instance().m_UserType)
	{
	case 1:// Пользователь
		ui.actObjects->setEnabled(false);
		ui.actPeriods->setEnabled(false);
		ui.actOrganisations->setEnabled(false);
		ui.actPosts->setEnabled(false);
		ui.actTariffs->setEnabled(false);
		ui.actPersons->setEnabled(false);
		ui.actPayments->setEnabled(false);
		ui.actUsers->setEnabled(false);
		ui.actPersons->setEnabled(false);
		ui.actPaymentsFio->setEnabled(false);
		ui.actDeductionsFio->setEnabled(false);
		break;
	default:
		break;
	}

	GetCloseDate();
    return 1;
}
	
void ZMainWindow::slotUpdate()
{
	//обновление открытых окон
	foreach (QMdiSubWindow *window, ui.mdiArea->subWindowList()) 
	{
		ZMdiChild *pChild = dynamic_cast<ZMdiChild *>(window->widget());
		if (pChild)
		{
			pChild->blockSignals(true);
			pChild->reload();
			pChild->blockSignals(false);
			continue;
		}
		ZViewGroups *pViewGroups = dynamic_cast<ZViewGroups*>(window->widget());
		if (pViewGroups)
		{
			pViewGroups->blockSignals(true);
			pViewGroups->UpdateSlot(0);
			pViewGroups->blockSignals(false);
			continue;
		}
	}
}

void ZMainWindow::slotOpenObjectsDialog()
{
	foreach (QMdiSubWindow *window, ui.mdiArea->subWindowList()) 
	{
		if (dynamic_cast<ZObjects*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZObjects*child = new ZObjects(this);
	connect(child, SIGNAL(needUpdate()), this,SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actObjects->text(), ui.actObjects->icon());
	child->init("objects");
	child->show();
}

void ZMainWindow::slotOpenPersonsDialog()
{
	foreach (QMdiSubWindow *window, ui.mdiArea->subWindowList()) 
	{
		if (dynamic_cast<ZPersons *>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild *child = new ZPersons(this);
	connect(child, SIGNAL(needUpdate()), this,SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actPersons->text(), ui.actPersons->icon());
	child->init("fio");
	child->show();
}

void ZMainWindow::slotOpenPeriodsDialog()
{
	foreach (QMdiSubWindow *window, ui.mdiArea->subWindowList()) 
	{
		if (dynamic_cast<ZPeriods *>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild *child = new ZPeriods(this);
	connect(child, SIGNAL(needUpdate()), this,SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actPeriods->text(), ui.actPeriods->icon());
	child->init("periods");
	child->show();
}

void ZMainWindow::slotOpenOrganisationsDialog()
{
	foreach (QMdiSubWindow *window, ui.mdiArea->subWindowList()) 
	{
		if (dynamic_cast<ZOrganisations *>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZOrganisations* child = new ZOrganisations(this);
	connect(child, SIGNAL(needUpdate()), this,SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actOrganisations->text(), ui.actOrganisations->icon());
	child->init("organisation");
	child->show();
}

void ZMainWindow::slotOpenVariantsDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZVariants*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZVariants* child = new ZVariants(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actVariants->text(), ui.actVariants->icon());
	child->init("variants");
	child->show();
}

void ZMainWindow::slotOpenPostsDialog()
{
	foreach (QMdiSubWindow *window, ui.mdiArea->subWindowList()) 
	{
		if (dynamic_cast<ZPosts*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild *child = new ZPosts(this);
	connect(child, SIGNAL(needUpdate()), this,SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actPosts->text(), ui.actPosts->icon());
	child->init("posts");
	child->show();
}

void ZMainWindow::slotOpenPaymentsDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZPayments*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild* child = new ZPayments(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actPayments->text(), ui.actPayments->icon());
	child->init("payments");
	child->show();
}


void ZMainWindow::slotOpenPaymentsFioDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZPayments2fio*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZPayments2fioBase* child = new ZPayments2fio(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->init("payments2fio");
	child->show();
}

void ZMainWindow::slotOpenDeductionsFioDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZDeductions2fio*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZPayments2fioBase* child = new ZDeductions2fio(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->init("payments2fio");
	child->show();
}

void ZMainWindow::slotOpenactContractsDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZContracts*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild* child = new ZContracts(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actContracts->text(), ui.actContracts->icon());
	child->init("contracts");
	child->show();
}

void ZMainWindow::slotOpenWorksDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZWorks*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild* child = new ZWorks(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actWorks->text(), ui.actWorks->icon());
	child->init("works");
	child->show();
}

void ZMainWindow::slotOpenEstimatesDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZEstimates*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild* child = new ZEstimates(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actEstimates->text(), ui.actEstimates->icon());
	child->init("estimates");
	child->show();
}

void ZMainWindow::slotOpenReportsDialog()
{
	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZReports*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZMdiChild* child = new ZReports(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actReports->text(), ui.actReports->icon());
	child->init("estimates");
	child->show();
}

void ZMainWindow::slotOpenConfigDialog()
{
	ZConfigForm dial(this);
	dial.exec();
}

void ZMainWindow::slotOpenUsersDialog()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("Введите пароль администратора"),
		tr("пароль:"), QLineEdit::Password, "", &ok);
	if (!ok || text.isEmpty() || !CheckPwd("администратор", text))
	{
		ZMessager::Instance().Message(_CriticalError, tr("Пароль неверный!"), tr("Ошибка"));
		return;
	}

	foreach(QMdiSubWindow * window, ui.mdiArea->subWindowList())
	{
		if (dynamic_cast<ZUsers*>(window->widget()))
		{
			ui.mdiArea->setActiveSubWindow(window);
			return;
		}
	}

	ZUsers* child = new ZUsers(this);
	connect(child, SIGNAL(needUpdate()), this, SLOT(slotUpdate()));
	ui.mdiArea->addSubWindow(child);
	child->setWindowTitleAndIcon(ui.actUsers->text(), ui.actUsers->icon());
	child->init("users");
	child->show();
}

void ZMainWindow::slotCleanMsg()
{
	ZMessager::Instance().Clear();
}

void ZMainWindow::slotSaveMsg()
{
	ZMessager::Instance().Save();
}
