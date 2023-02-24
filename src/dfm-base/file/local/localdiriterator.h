// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALDIRITERATOR_H
#define LOCALDIRITERATOR_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractdiriterator.h"

#include <dfm-io/core/denumerator.h>

#include <QDirIterator>
#include <QSharedPointer>

class QUrl;
namespace dfmbase {
class LocalFileInfo;
class LocalDirIteratorPrivate;

class LocalDirIterator : public AbstractDirIterator
{
    QScopedPointer<LocalDirIteratorPrivate> d;

public:
    explicit LocalDirIterator(const QUrl &url,
                              const QStringList &nameFilters = QStringList(),
                              QDir::Filters filters = QDir::NoFilter,
                              QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    virtual ~LocalDirIterator() override;
    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual void close() override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const AbstractFileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;
    virtual void cacheBlockIOAttribute() override;
    bool enableIteratorByKeyword(const QString &keyword) override
    {
        Q_UNUSED(keyword);
        return false;
    }
    void setArguments(const QMap<DFMIO::DEnumerator::ArgumentKey, QVariant> &argus);
    QList<QSharedPointer<DFMIO::DEnumerator::SortFileInfo>> sortFileInfoList();
};
}

#endif   // LOCALDIRITERATOR_H