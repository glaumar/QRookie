#include "resource_locator.h"

// Variável global para armazenar o prefixo de recursos
QString resourcePrefix = "";

QString resolvePrefix(QString cmd)
{
    if (resourcePrefix.isEmpty()) {
#ifdef MACOS
        resourcePrefix = QCoreApplication::applicationDirPath() + "/../Resources/";
#endif
    }

    QString resPath;
#ifdef MACOS
    resPath = resourcePrefix;
#else
    resPath = "";
#endif

    QString cmdPath = resPath + cmd;
    QFile adbFile(cmdPath);
#ifdef MACOS
    if (!adbFile.exists()) {
        qWarning() << cmd << " executable does not exist at:" << resPath;
    }
#endif

    return cmdPath;
}