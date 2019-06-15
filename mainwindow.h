#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MdiChild;
class QMdiSubWindow;
class QSignalMapper;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionnew_file_n_triggered();

    void updateMenus();

    MdiChild* createMdiChild();

    void setActiveSubWindow(QWidget* window);

    void on_actionopen_file_o_triggered();

    void updateWindowMenu();

    void on_actionsave_S_triggered();

    void on_actionundo_triggered();

    void on_actionclose_triggered();

    void on_actionexit_X_triggered();

    void showTextRowAndCol();    //显示文本的行号和列号

protected:
    void closeEvent(QCloseEvent *event);   //关闭事件

private:
    Ui::MainWindow *ui;

    QAction* actionSeparator;    //间隔器
    MdiChild* activeMdiChild();  //活动窗口
    QMdiSubWindow* findMdiChild(const QString &fileName);

    QSignalMapper* windowMapper;

    void readSettings();       //读取窗口设置
    void writeSettings();      //写入窗口设置

    void initWindow();
};

#endif // MAINWINDOW_H
