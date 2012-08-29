#ifndef DRIVERINFO_H
#define DRIVERINFO_H

/*  INDI Device
    Copyright (C) 2012 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

 */

#include <QObject>

#include <libindi/baseclient.h>
#include "indicommon.h"

class DriverManager;
class ServerManager;
class ClientManager;

class DriverInfo : public QObject
{
    Q_OBJECT

public:

    DriverInfo(const QString &inName);
      ~DriverInfo();

    void clear();
    QString getServerBuffer();

    const QString & getName() { return name; }
    void setName(const QString &newName) { name = newName; }

    void setTreeLabel(const QString &inTreeLabel) { treeLabel = inTreeLabel;}
    const QString &getTreeLabel() { return treeLabel;}

    void setUniqueLabel(const QString &inUniqueLabel) { uniqueLabel = inUniqueLabel; }
    const QString &getUniqueLabel() { return uniqueLabel; }

    void setDriver(const QString &newDriver) { driver = newDriver; }
    const QString &getDriver() { return driver; }

    void setVersion(const QString &newVersion) { version = newVersion; }
    const QString &getVersion() { return version; }

    void setType(DeviceFamily newType) { type = newType;}
    DeviceFamily getType() { return type;}

    void setDriverSource(DriverSource newDriverSource) { driverSource = newDriverSource;}
    DriverSource getDriverSource() { return driverSource; }

    void setServerManager(ServerManager *newServerManager) { serverManager = newServerManager;}
    ServerManager* getServerManager() { return serverManager; }

    void setClientManager(ClientManager *newClientManager) { clientManager = newClientManager;}
    ClientManager *getClientManager() { return clientManager; }

    void setUserPort(const QString &inUserPort) { userPort = inUserPort;}
    const QString & getUserPort() { return userPort;}

    void setServerState(bool inState);
    bool getServerState() { return serverState;}

    void setClientState(bool inState);
    bool getClientState() { return clientState; }

    void setHostParameters(const QString & inHost, const QString & inPort) { hostname = inHost; port = inPort; }
    const QString &getHost() { return hostname; }
    const QString &getPort() { return port; }

    void setBaseDevice(INDI::BaseDevice *idv) { baseDevice = idv;}
    INDI::BaseDevice* getBaseDevice() { return baseDevice; }

private:

    QString name;                       // Actual device name as defined by INDI server
    QString treeLabel;                  // How it appears in the GUI initially as read from source
    QString uniqueLabel;                // How it appears in INDI Menu in case tree_label above is taken by another device

    QString driver;                     // Exec for the driver
    QString version;                    // Version of the driver (optional)
    QString userPort;                   // INDI server port as the user wants it.

    QString port;                       // INDI Host port
    QString hostname;                   // INDI Host hostname

    DeviceFamily type;                  // Device type (Telescope, CCD..etc), if known (optional)

    bool serverState;                   // Is the driver in the server running?
    bool clientState;                   // Is the client connected to the server running the desired driver?

    DriverSource driverSource;          // How did we read the driver information? From XML file? From 3rd party file? ..etc.
    ServerManager *serverManager;       // Who is managing this device?
    ClientManager *clientManager;       // Any GUI client handling this device?

    INDI::BaseDevice *baseDevice;       // Pointer to base device, if any.

signals:
    void deviceStateChanged(DriverInfo *);
};


#endif // DRIVERINFO_H
