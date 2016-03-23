#include "dtoolbar.h"
#include "dicontextbutton.h"
#include "dcheckablebutton.h"
#include "dsearchbar.h"
#include "dtabbar.h"
#include "../app/fmevent.h"
#include "../app/global.h"
#include "dcrumbwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

const int DToolBar::ButtonHeight = 20;

DToolBar::DToolBar(QWidget *parent) : QFrame(parent)
{
    initData();
    initUI();
    initConnect();
}

DToolBar::~DToolBar()
{

}

void DToolBar::initData()
{

}

void DToolBar::initUI()
{
    initAddressToolBar();
    initContollerToolBar();

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_addressToolBar);
    mainLayout->addWidget(m_contollerToolBar);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

void DToolBar::initAddressToolBar()
{
    m_addressToolBar = new QFrame;
    m_addressToolBar->setObjectName("AddressToolBar");
    m_addressToolBar->setFixedHeight(40);

    m_backButton = new DStateButton(":/icons/images/icons/backward_normal.png", this);
    m_backButton->setObjectName("backButton");
    m_backButton->setFixedWidth(25);
    m_backButton->setFixedHeight(20);
    m_forwardButton = new DStateButton(":/icons/images/icons/forward_normal.png", this);
    m_forwardButton->setObjectName("forwardButton");
    m_forwardButton->setFixedWidth(25);
    m_forwardButton->setFixedHeight(20);
    m_upButton = new DStateButton(":/images/images/dark/appbar.arrow.up.png", this);

    m_searchBar = new DSearchBar;
    m_searchBar->setPlaceholderText("Enter address");
    m_searchBar->setAlignment(Qt::AlignHCenter);

    m_crumbWidget = new DCrumbWidget;

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_backButton);
    mainLayout->addWidget(m_forwardButton);
    mainLayout->addWidget(m_upButton);
    mainLayout->addWidget(m_crumbWidget);
    mainLayout->addWidget(m_searchBar);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);
    m_addressToolBar->setLayout(mainLayout);
}

void DToolBar::initContollerToolBar()
{
    m_contollerToolBar = new QFrame;
    m_contollerToolBar->setObjectName("ContollerToolBar");
    m_contollerToolBar->setFixedHeight(40);
    m_iconViewButton = new QPushButton(this);
    m_iconViewButton->setFixedHeight(20);
    m_iconViewButton->setFixedWidth(26);
    m_iconViewButton->setObjectName("iconViewButton");
    m_iconViewButton->setCheckable(true);

    m_listViewButton = new QPushButton(this);
    m_listViewButton->setFixedHeight(20);
    m_listViewButton->setFixedWidth(26);
    m_listViewButton->setObjectName("listViewButton");
    m_listViewButton->setCheckable(true);

    m_viewButtonGroup = new QButtonGroup(this);
    m_viewButtonGroup->addButton(m_iconViewButton, 0);
    m_viewButtonGroup->addButton(m_listViewButton, 1);

    m_sortingButton = new DStateButton(":/images/images/light/appbar.sort.png",
                                        this);
    m_sortingButton->setFixedHeight(20);
    m_sortingButton->setFixedWidth(26);
    m_sortingButton->setObjectName("sortingButton");

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconViewButton);
    mainLayout->addWidget(m_listViewButton);
    mainLayout->addWidget(m_sortingButton);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);
    m_contollerToolBar->setLayout(mainLayout);
}

void DToolBar::initConnect()
{
    connect(m_iconViewButton, &DStateButton::clicked, this, &DToolBar::requestIconView);
    connect(m_listViewButton, &DStateButton::clicked, this, &DToolBar::requestListView);
    connect(m_backButton, &DStateButton::clicked,this, &DToolBar::backButtonClicked);
    connect(m_searchBar, &DSearchBar::returnPressed, this, &DToolBar::searchBarTextEntered);
    connect(m_crumbWidget, &DCrumbWidget::crumbSelected, this, &DToolBar::crumbSelected);
    connect(m_upButton, SIGNAL(released()), this, SLOT(upButtonClicked()));
    connect(m_searchBar, SIGNAL(searchBarFocused()), this, SLOT(searchBarActivated()));
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged, this, &DToolBar::crumbChanged);
}

void DToolBar::searchBarActivated()
{
    m_crumbWidget->hide();
    m_searchBar->setAlignment(Qt::AlignLeft);
    m_searchBar->clear();
    QAction * action = m_searchBar->setClearAction();
    connect(action, &QAction::triggered, this, &DToolBar::searchBarDeactivated);
    disconnect(m_searchBar, SIGNAL(searchBarFocused()), this, SLOT(searchBarActivated()));
}

void DToolBar::searchBarDeactivated()
{
    m_crumbWidget->show();
    m_searchBar->clear();
    m_searchBar->setAlignment(Qt::AlignHCenter);
    QAction * action = m_searchBar->removeClearAction();
    disconnect(action, &QAction::triggered, this, &DToolBar::searchBarDeactivated);
    connect(m_searchBar, SIGNAL(searchBarFocused()), this, SLOT(searchBarActivated()));
}


/**
 * @brief DToolBar::searchBarTextEntered
 *
 * Set the tab bar when return press is detected
 * on search bar.
 */
void DToolBar::searchBarTextEntered()
{
    QString path = m_searchBar->text();
    qDebug() << path;

    FMEvent event;

    event.source = FMEvent::SearchLine;
    event.dir = path;

    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::crumbSelected(QString path)
{
    FMEvent event;

    event.source = FMEvent::SearchLine;
    event.dir = path;

    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::crumbChanged(const FMEvent &event)
{
    if(event.source == FMEvent::CrumbButton)
        return;

    qDebug() << "crumb chagned";
    m_crumbWidget->setCrumb(event.dir);
}

/**
 * @brief DToolBar::upButtonClicked
 *
 * Move or shrink to the given index of the tabs depending
 * on the amount of tabs shown. This will be triggered when
 * up button is clicked.
 */
void DToolBar::upButtonClicked()
{
    QString text = m_crumbWidget->back();

    FMEvent event;

    event.source = FMEvent::UpButton;
    event.dir = text;

    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::searchBarChanged(QString path)
{
    m_searchBar->setText(path);
}


