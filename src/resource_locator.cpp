#include "resource_locator.h"

// Variável global para armazenar o prefixo de recursos
QString resourcePrefix = "";

QString resolvePrefix(QString cmd)
{
#ifdef MACOS
    if (resourcePrefix.isEmpty()) {
        resourcePrefix = QCoreApplication::applicationDirPath() + "/../Resources/";
    }

    QString cmdPath = resourcePrefix + cmd;
    QFile adbFile(cmdPath);
    if (!adbFile.exists()) {
        qWarning() << cmd << " executable does not exist at:" << cmdPath;
    }

    return cmdPath;
#else
    return cmd;
#endif
}