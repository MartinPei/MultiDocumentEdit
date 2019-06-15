#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <mdichild.h>
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QSignalMapper>
#include <QSettings>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);

    updateMenus();

    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this,
            SLOT(updateMenus()));

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
    updateWindowMenu();
    connect(ui->menu_W, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

    readSettings();

    initWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionnew_file_n_triggered()
{
    MdiChild *child = createMdiChild();
    child->newFile();
    child->show();
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    ui->actionsave_S->setEnabled(hasMdiChild);
    ui->actionsave_as_A->setEnabled(hasMdiChild);
    ui->actionpaste_P->setEnabled(hasMdiChild);
    ui->actionclose->setEnabled(hasMdiChild);
    ui->actionclose_all_A->setEnabled(hasMdiChild);
    ui->actiontiling_T->setEnabled(hasMdiChild);
    ui->actioncascading_C->setEnabled(hasMdiChild);
    ui->actionnext_X->setEnabled(hasMdiChild);
    ui->actionprevious_Y->setEnabled(hasMdiChild);

    actionSeparator->setVisible(hasMdiChild);

    bool hasSelection = (activeMdiChild()&&activeMdiChild()->textCursor().hasSelection());
    ui->actioncut_T->setEnabled(hasSelection);
    ui->actioncopy_C->setEnabled(hasSelection);
    ui->actionundo->setEnabled(activeMdiChild()&&activeMdiChild()->document()->isUndoAvailable());
    ui->actionrecovery_R->setEnabled(activeMdiChild()&&activeMdiChild()->document()->isRedoAvailable());
}

MdiChild *MainWindow::createMdiChild()
{
    MdiChild* child = new MdiChild;
    ui->mdiArea->addSubWindow(child);

    connect(child, SIGNAL(copyAvailable(bool)), ui->actioncut_T, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)), ui->actioncopy_C, SLOT(setEnabled(bool)));
    connect(child->document(), SIGNAL(undoAvailable(bool)),ui->actionundo, SLOT(setEnabled(bool)));
    connect(child->document(), SIGNAL(redoAvailable(bool)), ui->actionrecovery_R, SLOT(setEnabled(bool)));

    connect(child, SIGNAL(cursorPositionChanged()), this, SLOT(showTextRowAndCol()));

    return child;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if(!window)
        return;
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

MdiChild *MainWindow::activeMdiChild()
{
    if(QMdiSubWindow* activeSubWindow = ui->mdiArea->activeSubWindow())
        return qobject_cast<MdiChild*>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString cannonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    foreach (QMdiSubWindow* window, ui->mdiArea->subWindowList()) {
        MdiChild* mdiChild = qobject_cast<MdiChild*>(window->widget());
        if(mdiChild->currentFile() == cannonicalFilePath)
            return window;
    }
    return 0;
}

void MainWindow::readSettings()
{
    QSettings settings("qt", "mdiEdit");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSettings()
{
    QSettings settings("qt", "mdiEdit");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

void MainWindow::initWindow()
{
    setWindowTitle(tr("多文档编辑器"));

    ui->mainToolBar->setWindowTitle(tr("工具栏"));

    //当多文档区域的内容超出可是区域后，出现滚动条
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->statusBar->showMessage(tr("欢迎使用多文档编辑器"));

    QLabel* label = new QLabel(this);
    label->setFrameStyle(QFrame::Box|QFrame::Sunken);
    label->setText(tr("<a href=\"http://www.yafeilinux.com\">yafeilinux.com</a>"));
    label->setTextFormat(Qt::RichText);
    label->setOpenExternalLinks(true);

    ui->statusBar->addPermanentWidget(label);
    ui->actionnew_file_n->setStatusTip(tr("创建一个文件"));
}

void MainWindow::on_actionopen_file_o_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty())
    {
        QMdiSubWindow* existing = findMdiChild(fileName);
        if(existing)
        {
            ui->mdiArea->setActiveSubWindow(existing);
            return;
        }

        MdiChild* child = createMdiChild();
        if(child->loadFile(fileName))
        {
            ui->statusBar->showMessage(tr("打开文件成功"), 2000);
            child->show();
        } else {
            child->show();
        }
    }
}

void MainWindow::updateWindowMenu()
{
    ui->menu_W->clear();
    ui->menu_W->addAction(ui->actionclose);
    ui->menu_W->addAction(ui->actionclose_all_A);
    ui->menu_W->addSeparator();
    ui->menu_W->addAction(ui->actiontiling_T);
    ui->menu_W->addAction(ui->actioncascading_C);
    ui->menu_W->addSeparator();
    ui->menu_W->addAction(ui->actionnext_X);
    ui->menu_W->addAction(ui->actionprevious_Y);
    ui->menu_W->addAction(actionSeparator);

    QList<QMdiSubWindow*> windows = ui->mdiArea->subWindowList();
    actionSeparator->setVisible(!windows.isEmpty());


    for (int i = 0; i < windows.size(); ++i) {
        MdiChild* child = qobject_cast<MdiChild*>(windows.at(i)->widget());
        QString text;
        if(i < 9)
        {
            text = tr("&%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
        } else{
            text = tr("%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
        }

        QAction* action = ui->menu_W->addAction(text);
        action->setCheckable(true);
        action->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
}

void MainWindow::on_actionsave_S_triggered()
{
    if(activeMdiChild() && activeMdiChild()->save())
        ui->statusBar->showMessage(tr("文件保存成功"), 2000);
}

void MainWindow::on_actionundo_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->undo();
}

void MainWindow::on_actionclose_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ui->mdiArea->closeAllSubWindows();
    if(ui->mdiArea->currentSubWindow())
        event->ignore();
    else{
        writeSettings();
        event->accept();
    }
}

void MainWindow::on_actionexit_X_triggered()
{
    qApp->closeAllWindows();
}

void MainWindow::showTextRowAndCol()
{
    if(activeMdiChild())
    {
        int rowNum = activeMdiChild()->textCursor().blockNumber()+1;
        int colNum = activeMdiChild()->textCursor().columnNumber()+1;

        ui->statusBar->showMessage(tr("%1 行 %2 列").arg(rowNum).arg(colNum), 2000);
    }
}





































