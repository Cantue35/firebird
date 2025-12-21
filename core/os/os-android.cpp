#include <QDebug>
#include <QJniEnvironment>
#include <QJniObject>
#include <QScopeGuard>
#include <QUrl>

#include "os.h"

// Obtain an Android Context without relying on Qt's internal Java helper classes.
// Using the standard Android API (ActivityThread.currentApplication) avoids
// compatibility issues across Qt for Android packaging variants.
static QJniObject android_context()
{
    // android.app.ActivityThread.currentApplication() -> android.app.Application
    QJniObject app = QJniObject::callStaticObjectMethod(
        "android/app/ActivityThread",
        "currentApplication",
        "()Landroid/app/Application;");

    if (!app.isValid())
        qWarning() << "Failed to obtain Android application context";

    return app;
}

static bool is_content_url(const char *path)
{
    constexpr const char pattern[] = "content:";
    return strncmp(pattern, path, sizeof(pattern) - 1) == 0;
}

// A handler to open android content:// URLs.
// Based on code by Florin9doi: https://github.com/nspire-emus/firebird/pull/94/files
FILE *fopen_utf8(const char *path, const char *mode)
{
    if (!is_content_url(path))
        return fopen(path, mode);

    // Android uses a "mode" string that differs from libc:
    // https://developer.android.com/reference/android/content/ContentResolver#openFileDescriptor(android.net.Uri,%20java.lang.String)
    QString android_mode;
    if (strcmp(mode, "rb") == 0)
        android_mode = QStringLiteral("r");
    else if (strcmp(mode, "r+b") == 0)
        android_mode = QStringLiteral("rw");
    else if (strcmp(mode, "wb") == 0)
        android_mode = QStringLiteral("rwt");
    else
        return nullptr;

    const QJniObject jpath = QJniObject::fromString(QString::fromUtf8(path));
    const QJniObject jmode = QJniObject::fromString(android_mode);

    const QJniObject uri = QJniObject::callStaticObjectMethod(
        "android/net/Uri",
        "parse",
        "(Ljava/lang/String;)Landroid/net/Uri;",
        jpath.object<jstring>());

    const QJniObject context = android_context();
    if (!context.isValid())
        return nullptr;

    const QJniObject contentResolver = context.callObjectMethod(
        "getContentResolver",
        "()Landroid/content/ContentResolver;");
    if (!contentResolver.isValid())
        return nullptr;

    // Call contentResolver.takePersistableUriPermission as we save the URI.
    // Intent.FLAG_GRANT_READ_URI_PERMISSION = 1
    // Intent.FLAG_GRANT_WRITE_URI_PERMISSION = 2
    int permflags = 1;
    if (android_mode.contains(QLatin1Char('w')))
        permflags |= 2;

    contentResolver.callMethod<void>(
        "takePersistableUriPermission",
        "(Landroid/net/Uri;I)V",
        uri.object<jobject>(),
        permflags);

    QJniEnvironment env;
    // QJniObject clears exceptions for its own calls, but direct JNIEnv usage needs handling.
    env.checkAndClearExceptions();

    const QJniObject parcelFileDescriptor = contentResolver.callObjectMethod(
        "openFileDescriptor",
        "(Landroid/net/Uri;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;",
        uri.object<jobject>(),
        jmode.object<jstring>());

    if (env.checkAndClearExceptions() || !parcelFileDescriptor.isValid())
        return nullptr;

    // Duplicate the file descriptor: detachFd() transfers ownership to us.
    const QJniObject parcelFileDescriptorDup = parcelFileDescriptor.callObjectMethod(
        "dup",
        "()Landroid/os/ParcelFileDescriptor;");

    if (env.checkAndClearExceptions() || !parcelFileDescriptorDup.isValid())
        return nullptr;

    const int fd = parcelFileDescriptorDup.callMethod<jint>("detachFd", "()I");
    if (fd < 0)
        return nullptr;

    return fdopen(fd, mode);
}

static QString android_basename_using_content_resolver(const QString &path)
{
    const QJniObject jpath = QJniObject::fromString(path);
    const QJniObject uri = QJniObject::callStaticObjectMethod(
        "android/net/Uri",
        "parse",
        "(Ljava/lang/String;)Landroid/net/Uri;",
        jpath.object<jstring>());

    const QJniObject context = android_context();
    if (!context.isValid())
        return {};

    const QJniObject contentResolver = context.callObjectMethod(
        "getContentResolver",
        "()Landroid/content/ContentResolver;");
    if (!contentResolver.isValid())
        return {};

    QJniEnvironment env;

    jclass openableColumnsClass = env.findClass("android/provider/OpenableColumns");
    if (!openableColumnsClass)
        return {};

    // OpenableColumns.DISPLAY_NAME is a String constant.
    const jfieldID displayNameField = env.findStaticField<jstring>(openableColumnsClass, "DISPLAY_NAME");
    if (!displayNameField)
        return {};

    jstring displayName = static_cast<jstring>(env->GetStaticObjectField(openableColumnsClass, displayNameField));
    if (env.checkAndClearExceptions())
        return {};

    const QJniObject col(displayName);

    jclass stringClass = env.findClass("java/lang/String");
    if (!stringClass)
        return {};

    jobjectArray projectionArray = env->NewObjectArray(1, stringClass, col.object<jstring>());
    if (env.checkAndClearExceptions() || !projectionArray)
        return {};

    const QJniObject projection(projectionArray);

    const QJniObject cursor = contentResolver.callObjectMethod(
        "query",
        "(Landroid/net/Uri;[Ljava/lang/String;Landroid/os/Bundle;Landroid/os/CancellationSignal;)Landroid/database/Cursor;",
        uri.object<jobject>(),
        projection.object<jobject>(),
        nullptr,
        nullptr);

    if (env.checkAndClearExceptions() || !cursor.isValid())
        return {};

    const auto closeCursor = qScopeGuard([&] { cursor.callMethod<void>("close", "()V"); });

    const bool hasContent = cursor.callMethod<jboolean>("moveToFirst", "()Z");
    if (env.checkAndClearExceptions() || !hasContent)
        return {};

    const QJniObject name = cursor.callObjectMethod("getString", "(I)Ljava/lang/String;", 0);
    if (!name.isValid())
        return {};

    return name.toString();
}

char *android_basename(const char *path)
{
    if (is_content_url(path)) {
        // Example: content://com.android.externalstorage.documents/document/primary%3AFirebird%2Fflash_tpad
        const QString pathStr = QString::fromUtf8(path);
        QString ret = android_basename_using_content_resolver(pathStr);

        // If that failed (e.g. because the permission expired), try to get something recognizable.
        if (ret.isEmpty()) {
            qWarning() << "Failed to get basename of" << pathStr << "using ContentResolver";
            const QStringList parts = pathStr.split(QStringLiteral("%2F"), Qt::SkipEmptyParts, Qt::CaseInsensitive);
            if (parts.size() > 1)
                ret = QUrl::fromPercentEncoding(parts.constLast().toUtf8());
        }

        if (!ret.isEmpty())
            return strdup(ret.toUtf8().constData());
    }

    return nullptr;
}
