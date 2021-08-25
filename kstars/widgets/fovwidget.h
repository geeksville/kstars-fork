/*
    fovwidget.h  -  description
    -------------------
    begin                : Sat 22 Sept 2007
    SPDX-FileCopyrightText: 2007 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#pragma once

#include <QFrame>

class FOV;

class FOVWidget : public QFrame
{
    Q_OBJECT
  public:
    explicit FOVWidget(QWidget *parent = nullptr);
    virtual ~FOVWidget() override = default;

    void setFOV(FOV *f);

  protected:
    void paintEvent(QPaintEvent *e) override;

  private:
    FOV *m_FOV { nullptr };
};
