/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "views/private/crumbbar_p.h"
#include "views/crumbbar.h"
#include "models/crumbmodel.h"
#include "utils/crumbmanager.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"

#include <DListView>

#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QMenu>
#include <QDebug>
#include <QGuiApplication>
#include <QClipboard>
#include <QPushButton>
#include <QMouseEvent>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static QString getIconName(const CrumbData &c)
{
    QString iconName = c.iconName;

    if (!iconName.isEmpty() && !iconName.startsWith("dfm_") && !iconName.contains("-symbolic"))
        iconName.append("-symbolic");

    return iconName;
}

/*!
 * \class IconItemDelegate
 * \brief
 */

IconItemDelegate::IconItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    setItemSpacing(10);
}

void IconItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.decorationAlignment = Qt::AlignCenter;
    DStyledItemDelegate::paint(painter, opt, index);
}

/*!
 * \class CrumbBarPrivate
 * \brief
 */

CrumbBarPrivate::CrumbBarPrivate(CrumbBar *qq)
    : q(qq)
{
    initData();
    initUI();
    initConnections();
}

CrumbBarPrivate::~CrumbBarPrivate()
{
}

/*!
 * \brief Remove all crumbs List items inside crumb listView.
 */
void CrumbBarPrivate::clearCrumbs()
{
    leftArrow.hide();
    rightArrow.hide();

    if (crumbModel)
        crumbModel->removeAll();
}

void CrumbBarPrivate::checkArrowVisiable()
{
    QScrollBar *scrollBar = crumbView.horizontalScrollBar();
    if (!scrollBar)
        return;

    leftArrow.setVisible(scrollBar->maximum() > 0);
    rightArrow.setVisible(scrollBar->maximum() > 0);

    leftArrow.setEnabled(scrollBar->value() != scrollBar->minimum());
    rightArrow.setEnabled(scrollBar->value() != scrollBar->maximum());
}

void CrumbBarPrivate::updateController(const QUrl &url)
{
    if (!crumbController || !crumbController->supportedUrl(url)) {
        if (crumbController) {
            crumbController->deleteLater();
        }
        crumbController = CrumbManager::instance()->createControllerByUrl(url);
        // Not found? Then nothing here...
        if (!crumbController) {
            qWarning() << "Unsupported url / scheme: " << url;
            // always has default controller
            crumbController = new CrumbInterface;
        }
        crumbController->setParent(q);
        QObject::connect(crumbController, &CrumbInterface::hideAddressBar, q, &CrumbBar::onHideAddressBar);
        QObject::connect(crumbController, &CrumbInterface::keepAddressBar, q, &CrumbBar::onKeepAddressBar);
        QObject::connect(crumbController, &CrumbInterface::hideAddrAndUpdateCrumbs, q, &CrumbBar::onHideAddrAndUpdateCrumbs);
    }
}

/*!
 * \brief Update the crumb controller hold by the crumb bar.
 *
 * \param url The url which controller should supported.
 */

void CrumbBarPrivate::setClickableAreaEnabled(bool enabled)
{
    if (clickableAreaEnabled == enabled) return;
    clickableAreaEnabled = enabled;
    q->update();
}

void CrumbBarPrivate::initUI()
{
    // Arrows
    QSize size(24, 24), iconSize(16, 16);

    leftArrow.setFocusPolicy(Qt::NoFocus);
    leftArrow.setIcon(QIcon::fromTheme("go-previous"));
    rightArrow.setIcon(QIcon::fromTheme("go-next"));
    rightArrow.setFocusPolicy(Qt::NoFocus);

    leftArrow.setFixedSize(size);
    leftArrow.setIconSize(iconSize);
    rightArrow.setFixedSize(size);
    rightArrow.setIconSize(iconSize);
    leftArrow.setFlat(true);
    rightArrow.setFlat(true);
    leftArrow.hide();
    rightArrow.hide();

    // Crumb List Layout
    crumbView.setObjectName("DCrumbListScrollArea");

    crumbView.setItemSpacing(10);
    crumbView.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbView.setFocusPolicy(Qt::NoFocus);
    crumbView.setContentsMargins(0, 0, 0, 0);
    crumbView.setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    crumbView.setIconSize({ 16, 16 });
    crumbView.setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    crumbView.setOrientation(QListView::LeftToRight, false);
    crumbView.setEditTriggers(QAbstractItemView::NoEditTriggers);
    crumbView.setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    crumbModel = new CrumbModel(q);
    crumbView.setModel(crumbModel);
    crumbView.setContextMenuPolicy(Qt::CustomContextMenu);

    // 点击listview空白可拖动窗口
    crumbView.viewport()->installEventFilter(q);
    crumbView.setItemDelegateForRow(0, new IconItemDelegate(&crumbView));

    // Crumb Bar Layout
    crumbBarLayout = new QHBoxLayout(q);
    crumbBarLayout->addWidget(&leftArrow);
    crumbBarLayout->addWidget(&crumbView);
    crumbBarLayout->addWidget(&rightArrow);
    crumbBarLayout->setContentsMargins(0, 0, 0, 0);
    crumbBarLayout->setSpacing(0);
    q->setLayout(crumbBarLayout);

    return;
}

void CrumbBarPrivate::initData()
{
    clickableAreaEnabled = Application::instance()->genericAttribute(Application::kShowCsdCrumbBarClickableArea).toBool();
}

void CrumbBarPrivate::initConnections()
{
    QObject::connect(&crumbView, &QListView::customContextMenuRequested,
                     q, &CrumbBar::onCustomContextMenu);

    QObject::connect(&crumbView, &QListView::clicked,
                     q, [=](const QModelIndex &index) {
                         if (index.isValid()) {
                             qInfo() << "sig send selectedUrl: " << index.data().toUrl();
                             emit q->selectedUrl(index.data(CrumbModel::FileUrlRole).toUrl());
                         }
                     });

    q->connect(&leftArrow, &QPushButton::clicked,
               q, [=]() {
                   crumbView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
               });

    q->connect(&rightArrow, &QPushButton::clicked,
               q, [=]() {
                   crumbView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
               });

    q->connect(crumbView.horizontalScrollBar(), &QScrollBar::valueChanged,
               q, [=]() {
                   checkArrowVisiable();
               });

    if (Application::instance()) {
        q->connect(Application::instance(),
                   &Application::csdClickableAreaAttributeChanged,
                   q, [=](bool enabled) {
                       setClickableAreaEnabled(enabled);
                   });
    }
}

/*!
 * \class CrumbBar
 * \inmodule dde-file-manager-lib
 *
 * \brief CrumbBar is the crumb bar widget of Deepin File Manager
 *
 * CrumbBar is the crumb bar widget of Deepin File Manager, provide the interface to manage
 * crumb bar state.
 *
 * \sa DFMCrumbInterface, DFMCrumbManager
 */

CrumbBar::CrumbBar(QWidget *parent)
    : QFrame(parent), d(new CrumbBarPrivate(this))
{
    setFrameShape(QFrame::NoFrame);
    setFixedHeight(36);
}

CrumbBar::~CrumbBar()
{
}

CrumbInterface *CrumbBar::controller() const
{
    return d->crumbController;
}

void CrumbBar::mousePressEvent(QMouseEvent *event)
{
    d->clickedPos = event->globalPos();

    if (event->button() == Qt::RightButton && d->clickableAreaEnabled) {
        event->accept();
        return;
    }

    QModelIndex index = d->crumbView.indexAt(event->pos());
    if (event->button() != Qt::RightButton || !index.isValid()) {
        QFrame::mousePressEvent(event);
    }
}

void CrumbBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (!d->clickableAreaEnabled) {
        return QFrame::mouseReleaseEvent(event);
    }

    QFrame::mouseReleaseEvent(event);
}

void CrumbBar::resizeEvent(QResizeEvent *event)
{
    d->checkArrowVisiable();

    return QFrame::resizeEvent(event);
}

void CrumbBar::showEvent(QShowEvent *event)
{
    //d->crumbListScrollArea.horizontalScrollBar()->setPageStep(d->crumbListHolder->width());
    d->crumbView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);

    d->checkArrowVisiable();

    return QFrame::showEvent(event);
}

bool CrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    QMouseEvent *me = nullptr;
    if (watched && watched->parent() == &d->crumbView && (me = dynamic_cast<QMouseEvent *>(event))) {
        QEvent::Type type = event->type();
        bool isMousePressed = type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick;
        static QPoint pos;
        if (isMousePressed) {
            pos = QCursor::pos();
        }

        bool isIgnore = isMousePressed || type == QEvent::MouseMove;
        if (isIgnore) {
            event->ignore();
            return true;
        }

        bool isDragging = (pos - QCursor::pos()).manhattanLength() > QApplication::startDragDistance();
        if (type == QEvent::MouseButtonRelease && me->button() == Qt::LeftButton && !isDragging) {
            QModelIndex index = d->crumbView.indexAt(me->pos());
            if (index.isValid() && index != d->crumbView.currentIndex()) {
                d->crumbView.clicked(index);
                return true;
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}

void CrumbBar::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = d->crumbView.indexAt(point);
    if (!index.isValid())
        return;
    qInfo() << "need add mouse right clicked menu show logic";
}

void CrumbBar::onUrlChanged(const QUrl &url)
{
    d->updateController(url);

    if (d->crumbController)
        d->crumbController->crumbUrlChangedBehavior(url);
}

void CrumbBar::onHideAddressBar()
{
    emit hideAddressBar();
}

void CrumbBar::onKeepAddressBar()
{
    // TODO(zhangs): impl me!
}

void CrumbBar::onHideAddrAndUpdateCrumbs(const QUrl &url)
{
    emit hideAddressBar();

    d->clearCrumbs();

    if (!d->crumbController) {
        qWarning("No controller found when trying to call DFMCrumbBar::updateCrumbs() !!!");
        qDebug() << "updateCrumbs (no controller) : " << url;
        return;
    }

    QList<CrumbData> &&crumbDataList = d->crumbController->seprateUrl(url);

    // create QStandardItem by crumb data
    for (const CrumbData &c : crumbDataList) {
        if (d->crumbModel) {
            QStandardItem *listitem = nullptr;
            if (c.iconName.isEmpty()) {
                listitem = new QStandardItem(c.displayText);
            } else {
                listitem = new QStandardItem(QIcon::fromTheme(getIconName(c)), QString());
            }

            listitem->setTextAlignment(Qt::AlignCenter);
            listitem->setCheckable(false);
            Q_ASSERT(c.url.isValid());
            listitem->setData(c.url, CrumbModel::Roles::FileUrlRole);
            d->crumbModel->appendRow(listitem);
        }
    }

    if (d->crumbView.selectionModel() && d->crumbModel) {
        qInfo() << d->crumbModel->lastIndex();
        d->crumbView.selectionModel()->select(d->crumbModel->lastIndex(), QItemSelectionModel::Select);
    }

    d->checkArrowVisiable();
    d->crumbView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);
}
