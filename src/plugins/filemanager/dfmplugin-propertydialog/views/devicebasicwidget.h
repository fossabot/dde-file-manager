/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DEVICEBASICWIDGET_H
#define DEVICEBASICWIDGET_H

#include "dfmplugin_propertydialog_global.h"
#include "dfm-base/interfaces/extendedcontrolview.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "services/common/propertydialog/property_defines.h"

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class DeviceBasicWidget : public DFMBASE_NAMESPACE::ExtendedControlDrawerView
{
    Q_OBJECT
public:
    explicit DeviceBasicWidget(QWidget *parent = nullptr);
    virtual ~DeviceBasicWidget() override;

private:
    void initUI();

public:
    virtual void setSelectFileUrl(const QUrl &url) override;

    void setSelectFileInfo(const DSC_NAMESPACE::DeviceInfo &info);

public slots:
    void slotFileDirSizeChange(qint64 size);

signals:
    void heightChanged(int height);

private:
    DFMBASE_NAMESPACE::KeyValueLabel *deviceType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *deviceTotalSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileSystem { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCount { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *freeSize { nullptr };
    QFrame *deviceInfoFrame { nullptr };
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // DEVICEBASICWIDGET_H
