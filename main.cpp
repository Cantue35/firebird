#include <QtGlobal>

#ifdef MOBILE_UI
  #include <QGuiApplication>
#else
  #include <QApplication>
  #include "mainwindow.h"
#endif

#include <QCoreApplication>
#include <QDebug>
#include <QLocale>
#include <QSettings>
#include <QTranslator>
#include <QVariantHash>

#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QtQml/qqml.h>

#include "emuthread.h"
#include "qtframebuffer.h"
#include "qmlbridge.h"
#include "kitmodel.h"

#if !defined(NO_TRANSLATION) && defined(IS_IOS_BUILD)
  #include <unistd.h>
  #include <sys/syscall.h>
#endif

static QVariantHash readOldSettings(const QString &org, const QString &app)
{
    QSettings settings(org, app);
    const auto keys = settings.allKeys();
    if (keys.isEmpty())
        return {};

    QVariantHash ret;
    for (const auto &key : keys)
        ret[key] = settings.value(key);
    return ret;
}

static void migrateSettings()
{
    QSettings current;
    if (current.value(QStringLiteral("version"), 0).toInt() != 0)
        return;

    qDebug("Trying to import old settings");

    QVariantHash old = readOldSettings(QStringLiteral("org.firebird"), QStringLiteral("firebird-emu"));
    if (old.isEmpty())
        old = readOldSettings(QStringLiteral("ndless"), QStringLiteral("firebird"));
    if (old.isEmpty())
        old = readOldSettings(QStringLiteral("ndless"), QStringLiteral("nspire_emu"));

    if (!old.isEmpty()) {
        for (auto it = old.begin(); it != old.end(); ++it)
            current.setValue(it.key(), it.value());
        qDebug("Settings imported");
    } else {
        qDebug("No previous settings found");
    }

    current.setValue(QStringLiteral("version"), 1);
    current.sync();
}

int main(int argc, char **argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #ifdef Q_OS_ANDROID
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
  #else
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  #endif
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#ifdef MOBILE_UI
    QGuiApplication app(argc, argv);

    QTranslator appTranslator;
    appTranslator.load(QLocale::system().name(), QStringLiteral(":/i18n/i18n/"));
    app.installTranslator(&appTranslator);
#else
    QApplication app(argc, argv);
    // Translator installed in MainWindow constructor (desktop)
#endif

#ifdef IS_IOS_BUILD
    app.setFont(QFont(QStringLiteral("Helvetica Neue")));
#endif

    QCoreApplication::setOrganizationDomain(QStringLiteral("firebird-emus.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("Firebird Emus"));
    QCoreApplication::setApplicationName(QStringLiteral("firebird-emu"));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaTypeStreamOperators<KitModel>();
#endif
    qRegisterMetaType<KitModel>();

    migrateSettings();

    // Bridge lifetime stack-owned for entire app lifetime
    QMLBridge qmlBridge;
    the_qml_bridge = &qmlBridge;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Preferred in Qt 6 register an existing instance as a singleton
    qmlRegisterSingletonInstance("Firebird.Emu", 1, 0, "Emu", &qmlBridge);
#else
    // Qt 5 fallback
    QQmlEngine::setObjectOwnership(&qmlBridge, QQmlEngine::CppOwnership);
    qmlRegisterSingletonType<QMLBridge>("Firebird.Emu", 1, 0, "Emu",
        [](QQmlEngine *, QJSEngine *) -> QObject * { return the_qml_bridge; });
#endif

    qmlRegisterType<QMLFramebuffer>("Firebird.Emu", 1, 0, "EmuScreen");
    qmlRegisterType<KitModel>("Firebird.Emu", 1, 0, "KitModel");

#ifndef MOBILE_UI
    MainWindow mw;
    main_window = &mw;
#else
    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qml/qml/"));
    engine.load(QUrl(QStringLiteral("qrc:/qml/qml/MobileUI.qml")));
#endif

    QObject::connect(&app, &QGuiApplication::lastWindowClosed, [&] {
        // QML ApplicationWindow may not count, so re-check visibility.
        for (auto *win : app.topLevelWindows())
            if (win && win->isVisible())
                return;
        emu_thread.stop();
    });

    const int execRet = app.exec();

#if !defined(NO_TRANSLATION) && defined(IS_IOS_BUILD)
    syscall(SYS_ptrace, 31 /* PT_DENY_ATTACH */, 0, NULL, 0);
#endif

    return execRet;
}
