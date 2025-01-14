// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALVAULTHELPERPLUGIN_H
#define VIRTUALVAULTHELPERPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "vaulthelperreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {



class VirtualVaultHelperPlugin : public dpf::Plugin
{
    Q_OBJECT
    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

    DPF_EVENT_REG_SLOT(slot_VaultHelper_ConnectLockScreenDBus)
    DPF_EVENT_REG_SLOT(slot_VaultHelper_TransparentUnlockVault)
    DPF_EVENT_REG_SLOT(slot_VaultHelper_PasswordFromKeyring)
    DPF_EVENT_REG_SLOT(slot_VaultHelper_SavePasswordToKeyring)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<VaultHelperReceiver> eventReceiver { new VaultHelperReceiver};
};

}

#endif   // VIRTUALVAULTHELPERPLUGIN_H
