#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QString>
#include <QDragEnterEvent>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:

    Ui::MainWindow *ui;
    QPushButton *btnBrowse;
    QListWidget *listBoxLocal;
    QListWidget *listBoxWorld;
    QPushButton *btnOk;
    QPushButton *btnDel;

    QString selectedPath;
    QString localSelection;
    QString worldSelection;

    QAction *actionColorizeItems;
    QAction *actionHideRegexMatches;

    QDialog *helpDialog = nullptr;

private slots:
    void browseFolder();
    void loadFolders(const QString &selectedPath);
    void onSelectionChanged();
    void createBackupAndReplace();
    bool validateFolder(const QString &folderPath);
    bool validateLocalDir(const QString &localPath);
    bool validateWorldDir(const QString &worldPath);
    bool copyNewFiles(const QString &localSelection, const QString &worldSelection);
    bool copyDirectory(const QString &sourceFolder, const QString &destFolder);
    bool backupOldFiles();
    void applyRegexToListBox(QListWidget *listBox, bool apply, bool hide);
    void colorizeItems(QListWidget *listBox, bool apply);
    void deleteSelectedItems();
    void showAboutDialog();
    void showHelpDialog();

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void onFolderDropped(const QString &folderPath);

};
#endif // MAINWINDOW_H
