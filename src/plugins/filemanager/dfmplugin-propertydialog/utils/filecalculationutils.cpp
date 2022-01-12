#include "filecalculationutils.h"

#include <QDebug>
#include <QThread>

#include <sys/vfs.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>
#include <sys/utsname.h>

DPPROPERTYDIALOG_USE_NAMESPACE
const static int kDefaultMemoryPageSize { 4096 };
FileCalculationUtils::FileCalculationUtils(QObject *parent)
    : QObject(parent)
{
    thread = new QThread();
    moveToThread(thread);
    connect(this, &FileCalculationUtils::sigStartRun, this, &FileCalculationUtils::statisticsFileInfo);
    thread->start();
}

FileCalculationUtils::~FileCalculationUtils()
{
    thread->wait();
    thread->terminate();
    if (info) {
        delete info;
        info = nullptr;
    }

    if (thread->isFinished())
        delete thread;
}

void FileCalculationUtils::startThread(const QList<QUrl> &files)
{
    emit sigStartRun(files);
}

void FileCalculationUtils::statisticsFileInfo(const QList<QUrl> &files)
{
    if (files.isEmpty())
        return;
    info = new FileStatisticInfo;

    for (QUrl url : files) {
        statisticFilesSize(url, info);
    }

    delete info;
    info = nullptr;
}

quint16 FileCalculationUtils::getMemoryPageSize()
{
    static const quint16 memoryPageSize = static_cast<quint16>(getpagesize());
    return memoryPageSize > 0 ? memoryPageSize : kDefaultMemoryPageSize;
}

void FileCalculationUtils::statisticFilesSize(const QUrl &url, FileStatisticInfo *&sizeInfo)
{
    char *paths[2] = { nullptr, nullptr };
    paths[0] = strdup(url.path().toUtf8().toStdString().data());
    FTS *fts = fts_open(paths, 0, nullptr);
    if (paths[0])
        free(paths[0]);

    if (nullptr == fts) {
        perror("fts_open");
        qWarning() << "fts_open open error : " << QString::fromLocal8Bit(strerror(errno));
        return;
    }

    int flg = 0;

    while (1) {
        FTSENT *ent = fts_read(fts);
        if (ent == nullptr) {
            break;
        }
        unsigned short flag = ent->fts_info;
        if (flag != FTS_DP) {
            sizeInfo->totalSize += ent->fts_statp->st_size <= 0 ? getMemoryPageSize() : ent->fts_statp->st_size;
        }

        if (flag == FTS_F) {
            sizeInfo->fileCount++;
        }

        if (flg == 1000) {
            emit sigTotalChange(sizeInfo->totalSize);
            emit sigFileChange(sizeInfo->fileCount);
            flg = 0;
        }
        flg++;
    }
    emit sigTotalChange(sizeInfo->totalSize);
    emit sigFileChange(sizeInfo->fileCount);
    fts_close(fts);
}
