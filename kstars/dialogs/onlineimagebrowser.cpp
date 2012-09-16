#include "onlineimagebrowser.h"

OnlineImageBrowser_ui::OnlineImageBrowser_ui(QWidget *parent)
    : QFrame(parent)
{
    setupUi(this);
}

OnlineImageBrowser::OnlineImageBrowser(QWidget *parent)
    : KDialog(parent)
{
    m_Ui = new OnlineImageBrowser_ui(this);
}
