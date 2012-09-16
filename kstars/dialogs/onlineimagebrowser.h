#ifndef ONLINEIMAGEBROWSER_H
#define ONLINEIMAGEBROWSER_H

#include "ui_onlineimagebrowser.h"

#include "KDialog"
#include <QFrame>

class OnlineImageBrowser_ui :public QFrame, public Ui::OnlineImageBrowser
{
public:
    OnlineImageBrowser_ui(QWidget *parent = 0);
};

class OnlineImageBrowser : public KDialog
{
public:
    OnlineImageBrowser(QWidget *parent = 0);


private:
    OnlineImageBrowser_ui *m_Ui;

};

#endif // ONLINEIMAGEBROWSER_H
