#ifndef OPSASTROBINAPI_H
#define OPSASTROBINAPI_H

#include "ui_opsastrobinapi.h"

#include "QFrame"

class OpsAstroBinApi : public QFrame, public Ui::OpsAstroBinApi
{
    Q_OBJECT

public:
    OpsAstroBinApi(QWidget *parent = 0);
};

#endif // OPSASTROBINAPI_H
