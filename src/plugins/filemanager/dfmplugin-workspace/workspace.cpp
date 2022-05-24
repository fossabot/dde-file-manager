/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "workspace.h"
#include "views/workspacewidget.h"
#include "views/fileview.h"
#include "views/renamebar.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"
#include "events/workspaceeventreceiver.h"
#include "events/workspaceunicastreceiver.h"
#include "menus/workspacemenuscene.h"
#include "menus/sortanddisplaymenuscene.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/common/propertydialog/propertydialogservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE

DPWORKSPACE_USE_NAMESPACE

namespace GlobalPrivate {
static DSB_FM_NAMESPACE::WindowsService *windowService { nullptr };
}   // namespace GlobalPrivate

void Workspace::initialize()
{
    DSB_FM_USE_NAMESPACE
    DFMBASE_USE_NAMESPACE

    ViewFactory::regClass<FileView>(Global::kFile);

    auto &ctx = dpfInstance.serviceContext();
    Q_ASSERT_X(ctx.loaded(WindowsService::name()), "Workspace", "WindowService not loaded");

    QString errStr;
    if (!ctx.load(PropertyDialogService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    GlobalPrivate::windowService = ctx.service<WindowsService>(WindowsService::name());
    connect(GlobalPrivate::windowService, &WindowsService::windowOpened, this, &Workspace::onWindowOpened, Qt::DirectConnection);
    connect(GlobalPrivate::windowService, &WindowsService::windowClosed, this, &Workspace::onWindowClosed, Qt::DirectConnection);
    WorkspaceUnicastReceiver::instance()->connectService();
    MenuService::service()->registerScene(WorkspaceMenuCreator::name(), new WorkspaceMenuCreator());
    MenuService::service()->registerScene(SortAndDisplayMenuCreator::name(), new SortAndDisplayMenuCreator());
    MenuService::service()->bind(SortAndDisplayMenuCreator::name(), WorkspaceMenuCreator::name());
}

bool Workspace::start()
{
    DSB_FM_USE_NAMESPACE
    DFMBASE_USE_NAMESPACE

    WorkspaceEventReceiver::instance()->initConnection();

    const QString &scheme = Global::kFile;

    if (WorkspaceHelper::instance()->isRegistedTopWidget(scheme)) {
        qWarning() << "custom top widget sechme " << scheme << "has been resigtered!";
        return false;
    }

    WorkspaceHelper::instance()->registerTopWidgetCreator(scheme, []() {
        CustomTopWidgetInterface *interface { new CustomTopWidgetInterface };
        interface->registeCreateTopWidgetCallback([]() {
            return new RenameBar();
        });
        interface->setKeepShow(false);
        return interface;
    });

    return true;
}

dpf::Plugin::ShutdownFlag Workspace::stop()
{
    return kSync;
}

void Workspace::onWindowOpened(quint64 windId)
{
    DFMBASE_USE_NAMESPACE

    auto window = GlobalPrivate::windowService->findWindowById(windId);
    Q_ASSERT_X(window, "WorkSpace", "Cannot find window by id");
    WorkspaceWidget *workspace = new WorkspaceWidget;
    WorkspaceHelper::instance()->addWorkspace(windId, workspace);
    window->installWorkSpace(workspace);

    connect(window, &FileManagerWindow::reqActivateNextTab, workspace, &WorkspaceWidget::onNextTab);
    connect(window, &FileManagerWindow::reqActivatePreviousTab, workspace, &WorkspaceWidget::onPreviousTab);
    connect(window, &FileManagerWindow::reqCloseCurrentTab, workspace, &WorkspaceWidget::onCloseCurrentTab);
    connect(window, &FileManagerWindow::reqActivateTabByIndex, workspace, &WorkspaceWidget::onSetCurrentTabIndex);
}

void Workspace::onWindowClosed(quint64 windId)
{
    WorkspaceHelper::instance()->removeWorkspace(windId);
}
