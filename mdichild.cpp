#include "mdichild.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QCloseEvent>
#include <QPushButton>
#include <QMenu>

MdiChild::MdiChild(QWidget* parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    isUntitled = true;
}

void MdiChild::newFile()
{
    //设置窗口编号，该编号一直被保存
    static int sequenceNumber = 1;

    isUntitled = true;

    curFile = tr("未命名文档%1.txt").arg(sequenceNumber++);

    setWindowTitle(curFile + "[*]" + tr("-多文档编辑器"));

    connect(document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
}

bool MdiChild::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::warning(this, tr("多文档编辑器"),
                             tr("无法读取文件%1:\n%s.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    connect(document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));
    return true;
}

bool MdiChild::save()
{
    if(isUntitled)
        return saveAs();
    else {
        return saveFile(curFile);
    }
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), curFile);
    if(fileName.isEmpty())
        return false;
    return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly|QFile::Text))
    {
        QMessageBox::warning(this, tr("多文档编辑器"),
                             tr("无法写入文件%1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return QFileInfo(curFile).fileName();
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    if(maybeSave())
        event->accept();
    else {
        event->ignore();
    }
}

void MdiChild::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu* menu = new QMenu;
    QAction* undo = menu->addAction(tr("撤销(&U)"), this, SLOT(undo()), QKeySequence::Undo);
    undo->setEnabled(document()->isUndoAvailable());

    QAction* redo = menu->addAction(tr("恢复(&R)"), this, SLOT(redo()), QKeySequence::Redo);
    redo->setEnabled(document()->isRedoAvailable());

    menu->addSeparator();

    QAction* cut = menu->addAction(tr("剪切(&T)"), this, SLOT(cut()), QKeySequence::Cut);
    cut->setEnabled(textCursor().hasSelection());

    QAction* copy = menu->addAction(tr("复制(&C)"), this, SLOT(copy()), QKeySequence::Copy);
    copy->setEnabled(textCursor().hasSelection());

    menu->addAction(tr("粘贴(&P)"), this, SLOT(paste()), QKeySequence::Paste);

    QAction* clear = menu->addAction(tr("清空"), this, SLOT(clear()));
    clear->setEnabled(!document()->isEmpty());

    menu->addSeparator();

    QAction* select = menu->addAction(tr("全选"), this, SLOT(selectAll()), QKeySequence::SelectAll);
    select->setEnabled(!document()->isEmpty());

    menu->exec(e->globalPos());

    delete menu;
}

void MdiChild::documentWasModified()
{
    setWindowModified(document()->isModified());
}

bool MdiChild::maybeSave()
{
    if(document()->isModified())
    {
        QMessageBox box;
        box.setWindowTitle(tr("多文档编辑器"));
        box.setText(tr("是否保存对“%1”的更改?")
                    .arg(userFriendlyCurrentFile()));
        box.setIcon(QMessageBox::Warning);
        QPushButton *yesBtn = box.addButton(tr("是(&Y)"), QMessageBox::YesRole);
        box.addButton(tr("否(&N)"), QMessageBox::NoRole);
        QPushButton *cancelBtn = box.addButton(tr("取消"),
                                               QMessageBox::RejectRole);
        box.exec();
        if(box.clickedButton() == yesBtn)
            return save();
        else if(box.clickedButton() == cancelBtn)
            return false;
    }
    return true;
}

void MdiChild::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    document()->setModified(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile()+"[*]");
}



























