#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDirIterator>
#include <QProcess>
#include <QRegularExpression>
#include <QLabel>
#include <QSettings>
#include <QTranslator>

static QRegularExpression getRegex() {
    static QRegularExpression regex("^[\\d]{4}\\.[\\d]{2}\\.[\\d]{2}-[\\d]{2}\\.[\\d]{2}\\.[\\d]{2}$");
    return regex;
}


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
, ui(new Ui::MainWindow) 
{
    ui->setupUi(this);

    this->setWindowTitle(tr("幻兽帕鲁回档助手"));

    QMenuBar *menuBar = new QMenuBar(this);

    this->setMenuBar(menuBar);


    QMenu *menu = menuBar->addMenu(tr("选项"));
    QAction *actionEnableButton = new QAction(tr("删除/回档"), this);
    actionEnableButton->setCheckable(true);
    menu->addAction(actionEnableButton);

    actionColorizeItems = new QAction(tr("备份颜色区分"), this);
    actionColorizeItems->setCheckable(true);
    actionColorizeItems->setChecked(true);
    menu->addAction(actionColorizeItems);

    actionHideRegexMatches = new QAction(tr("隐藏手动存档备份"), this); 
    actionHideRegexMatches->setCheckable(true);
    menu->addAction(actionHideRegexMatches);
    actionHideRegexMatches->setChecked(true); 

    QAction *helpAction = new QAction(tr("帮助"), this);
    menuBar->addAction(helpAction);
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelpDialog);

    QAction *aboutAction = new QAction(tr("关于"), this);
    menuBar->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    btnBrowse = new QPushButton(tr("打开存档目录"), this);
    listBoxLocal = new QListWidget(this);
    listBoxWorld = new QListWidget(this);
    btnOk = new QPushButton(tr("开始回档"), this);
    btnOk->setStyleSheet("QPushButton { background-color: green; color: white; }");
    btnDel = new QPushButton(tr("删除选中存档"), this);
    btnOk->setDisabled(true);
    btnDel->setDisabled(true);
    btnDel->setStyleSheet("QPushButton { background-color: red; color: white; }");
    btnDel->hide(); 


    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *labelLocal = new QLabel("Local", this);
    QLabel *labelWorld = new QLabel("World", this);
    labelLocal->setAlignment(Qt::AlignCenter);
    labelWorld->setAlignment(Qt::AlignCenter);

    QVBoxLayout *localLayout = new QVBoxLayout;
    localLayout->addWidget(listBoxLocal);
    localLayout->addWidget(labelLocal);

    QVBoxLayout *worldLayout = new QVBoxLayout;
    worldLayout->addWidget(listBoxWorld); 
    worldLayout->addWidget(labelWorld);

    QHBoxLayout *listBoxLayout = new QHBoxLayout;
    listBoxLayout->addLayout(localLayout);
    listBoxLayout->addLayout(worldLayout);

    layout->addWidget(btnBrowse);
    layout->addLayout(listBoxLayout);
    layout->addWidget(btnOk);
    layout->addWidget(btnDel);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(btnBrowse, &QPushButton::clicked, this, &MainWindow::browseFolder);
    connect(listBoxLocal, &QListWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    connect(listBoxWorld, &QListWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    connect(btnOk, &QPushButton::clicked, this, &MainWindow::createBackupAndReplace);

    connect(actionEnableButton, &QAction::toggled, btnDel, &QPushButton::setEnabled);

    connect(actionColorizeItems, &QAction::toggled, this, [this](bool checked) {
        colorizeItems(this->listBoxLocal, checked);
        colorizeItems(this->listBoxWorld, checked);
    });

    connect(actionHideRegexMatches, &QAction::toggled, this, [this](bool checked) {
        applyRegexToListBox(listBoxLocal, checked, true);
        applyRegexToListBox(listBoxWorld, checked, true);
    });

  
    connect(actionEnableButton, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            this->btnDel->show();
            this->btnOk->hide();
        } else {
            this->btnDel->hide();
            this->btnOk->show();
        }
    });

    connect(btnDel, &QPushButton::clicked, this, &MainWindow::deleteSelectedItems);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browseFolder() {
    QString user_profile = QDir::homePath();
    QString initial_path = QDir::cleanPath(user_profile + "/AppData/Local/Pal/Saved/SaveGames");
    
    QString selected_folder = QFileDialog::getExistingDirectory(nullptr, tr("打开存档目录"), initial_path);

    if (!selected_folder.isEmpty()) {
        if (validateFolder(selected_folder)) {
            selectedPath = selected_folder;
            MainWindow::folderSelected = selected_folder;
            loadFolders(MainWindow::folderSelected);
        } else {
            QMessageBox::information(this, tr("异常"), tr("选择的文件夹异常，请重新检查目录是否正确。"));
            // std::cerr << "Selected folder is not valid." << std::endl;
        }
    }
}

void MainWindow::loadFolders(const QString &selectedPath) {
    QDir dir(selectedPath);
    listBoxLocal->clear();
    listBoxWorld->clear();

    QString localPath = dir.filePath("backup/local");
    QString worldPath = dir.filePath("backup/world");

    if (QDir(localPath).exists()) {
        QStringList localFolders;
        QDirIterator localIt(localPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
        while (localIt.hasNext()) {
            localIt.next();
            localFolders << localIt.fileName();
        }
        std::sort(localFolders.begin(), localFolders.end(), std::greater<>());
        for (const QString &folder : localFolders) {
            listBoxLocal->addItem(folder);
        }
    }

    if (QDir(worldPath).exists()) {
        QStringList worldFolders;
        QDirIterator worldIt(worldPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
        while (worldIt.hasNext()) {
            worldIt.next();
            worldFolders << worldIt.fileName();
        }
        std::sort(worldFolders.begin(), worldFolders.end(), std::greater<>());
        for (const QString &folder : worldFolders) {
            listBoxWorld->addItem(folder);
        }
    }

    if (actionColorizeItems && actionColorizeItems->isChecked()) {
        colorizeItems(listBoxLocal, true);
        colorizeItems(listBoxWorld, true);
    }

    if (actionHideRegexMatches && actionHideRegexMatches->isChecked()) {
        applyRegexToListBox(listBoxLocal, true, true);
        applyRegexToListBox(listBoxWorld, true, true);
    }

}

void MainWindow::onSelectionChanged(){
    if (!listBoxLocal->selectedItems().isEmpty()) {
        localSelection = listBoxLocal->currentItem()->text();
    }

    if (!listBoxWorld->selectedItems().isEmpty()) {
        worldSelection = listBoxWorld->currentItem()->text();
    }

    if (!listBoxLocal->selectedItems().isEmpty() && !listBoxWorld->selectedItems().isEmpty()) {
        btnOk->setEnabled(true);
    } else {
        btnOk->setDisabled(true);
    }
}

bool MainWindow::validateFolder(const QString &selectedPath) {
    QDir dir(selectedPath);
    if (!dir.exists("backup")) {
        return false;
    }
    return true;
}

bool MainWindow::validateLocalDir(const QString &localPath) {
    if (!QFile::exists(QDir(localPath).filePath("LocalData.sav"))) {
        QMessageBox::critical(this, "Error", tr("请检查目标文件夹是否包含LocalData.sav文件."));
        return false;
    }
    return true;
}

bool MainWindow::validateWorldDir(const QString &worldPath) {
    QDir dir(worldPath);
    if (!dir.exists("Players") || !QFile::exists(dir.filePath("Level.sav")) || !QFile::exists(dir.filePath("LevelMeta.sav"))) {
        QMessageBox::critical(this, "Error", tr("请检查目标文件夹是否包含正确文件."));
        return false;
    }
    return true;
}

void MainWindow::createBackupAndReplace() {
    if (localSelection.left(16) != worldSelection.left(16)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("检查"), tr("两侧时间看起来不一致，确定吗？"),
                                      QMessageBox::Ok | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel) {
            return; 
        }
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("确认替换"),
                                      tr("将尝试备份当前存档，并更换进度 \n\n是否继续?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    bool bakSuccess =backupOldFiles();
    loadFolders(MainWindow::folderSelected);
    if (!bakSuccess){
        QMessageBox::StandardButton keepReply;
        keepReply = QMessageBox::question(this, tr("确认继续"),
                                      tr("备份过程发现存档文件不全，当前存档已清理备份. \n继续执行将尝试更换选定存档 \n\n是否继续?"),
                                      QMessageBox::Yes | QMessageBox::No);

        if(keepReply == QMessageBox::No){
            return;
        }
    }

    if (!copyNewFiles(localSelection, worldSelection)) {
        QMessageBox::critical(this, "Error", tr("无法复制新文件。"));
        return;
    }

    QMessageBox::information(this, "Success", tr("存档已成功替换。"));
}


bool MainWindow::copyNewFiles(const QString &localSelection, const QString &worldSelection) {
    QString sourceLocalPath = selectedPath + "/backup/local/" + localSelection + "/LocalData.sav";
    QString targetLocalPath = selectedPath + "/LocalData.sav";

    QString sourceWorldPath = selectedPath + "/backup/world/" + worldSelection + "/";
    QString targetWorldPath = selectedPath + "/";

    if (!QFile::copy(sourceLocalPath, targetLocalPath)) {
        QMessageBox::critical(this, "Error", tr("复制 LocalData.sav 失败。"));
        return false;
    }

    QDir worldDir(sourceWorldPath);
    if (!worldDir.exists()) {
        QMessageBox::critical(this, "Error", tr("源 World 文件夹不存在。"));
        return false;
    }

    copyDirectory(sourceWorldPath, targetWorldPath);
    return true;
}

QString getCustomDirName() {
    QDateTime now = QDateTime::currentDateTime();
    qint64 timestamp = now.toMSecsSinceEpoch();
    QString formattedTime = now.toString("yyyy.MM.dd-HH.mm.ss");
    return formattedTime + "-" + QString::number(timestamp);
}


bool MainWindow::backupOldFiles() {
    QString dirName = getCustomDirName();
    QString localBackupPath = selectedPath + "/backup/local/" + dirName;
    QString worldBackupPath = selectedPath + "/backup/world/" + dirName;

    QDir().mkpath(localBackupPath);
    QDir().mkpath(worldBackupPath);

    QStringList localFiles = {selectedPath + "/LocalData.sav"};
    QStringList worldFiles = {selectedPath + "/Players", selectedPath + "/Level.sav", selectedPath + "/LevelMeta.sav"};

    bool success = true;
    QStringList missingFiles;

    for (const auto& file : localFiles) {
        QFile f(file);
        if (f.exists()) {
            success &= f.rename(localBackupPath + "/" + QFileInfo(file).fileName());
        } else {
            missingFiles << file;
            success = false;
        }
    }

    for (const auto& file : worldFiles) {
        QFile f(file);
        if (f.exists()) {
            success &= f.rename(worldBackupPath + "/" + QFileInfo(file).fileName());
        } else {
            missingFiles << file;
            success = false;
        }
    }
    if (!missingFiles.isEmpty()) {
        QMessageBox::warning(nullptr, tr("缺失文件"), tr("缺失清单:\n") + missingFiles.join("\n"));
    }

    return success;
}

bool MainWindow::copyDirectory(const QString &sourceFolder, const QString &destFolder) {
    QDir sourceDir(sourceFolder);
    if (!sourceDir.exists())
        return false;

    QDir destDir(destFolder);
    if (!destDir.exists()) {
        destDir.mkdir(destFolder);
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for (const QString &file : files) {
        QString srcName = sourceFolder + '/' + file;
        QString destName = destFolder + '/' + file;
        QFile::copy(srcName, destName);
    }

    QStringList directories = sourceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &directory : directories) {
        QString srcName = sourceFolder + '/' + directory;
        QString destName = destFolder + '/' + directory;
        copyDirectory(srcName, destName);
    }

    return true;
}

void MainWindow::applyRegexToListBox(QListWidget *listBox, bool apply, bool hide) {
    // QRegularExpression regex(".*-\\d{13}");
    QRegularExpression regex = getRegex();
    for (int i = 0; i < listBox->count(); ++i) {
        QListWidgetItem *item = listBox->item(i);
        if (!regex.match(item->text()).hasMatch()) {
            if (hide) {
                item->setHidden(apply);
            } else {
                item->setForeground(apply ? Qt::gray : Qt::black);
            }
        }
    }
}


void MainWindow::colorizeItems(QListWidget *listBox, bool apply) {
    // QRegularExpression regex(".*-\\d{13}");
    QRegularExpression regex = getRegex();
    for (int i = 0; i < listBox->count(); ++i) {
        QListWidgetItem *item = listBox->item(i);
        if (apply) {
            item->setBackground(!regex.match(item->text()).hasMatch() ? QBrush(Qt::red) : QBrush(Qt::green));
        } else {
            item->setBackground(QBrush(Qt::white));  // 或者您希望恢复的默认颜色
        }
    }
}

void MainWindow::deleteSelectedItems() {
    QListWidgetItem* itemLocal = listBoxLocal->currentItem();
    QListWidgetItem* itemWorld = listBoxWorld->currentItem();


    QString localFolderPath = selectedPath + "/backup/local/" + (itemLocal ? itemLocal->text() : "");
    QString worldFolderPath = selectedPath + "/backup/world/" + (itemWorld ? itemWorld->text() : "");

    if (itemLocal == nullptr && itemWorld == nullptr) {
        QMessageBox::warning(this, tr("删除确认"), tr("没有选中任何项目！"));
        return;
    }
    if (localSelection.left(16) != worldSelection.left(16)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("检查"), tr("两侧时间看起来不一致，确定吗？"),
                                      QMessageBox::Ok | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel) {
            return; 
        }
    }


    QString warningText = tr("您将要删除以下目录：\n");
    if (itemLocal) warningText += localFolderPath + "\n";
    if (itemWorld) warningText += worldFolderPath + "\n";
    warningText += tr("这个操作是不可逆的，您确定要继续吗？");

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("确认删除"), warningText, QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (itemLocal && QDir(localFolderPath).exists()) {
            QDir(localFolderPath).removeRecursively();
        }
        if (itemWorld && QDir(worldFolderPath).exists()) {
            QDir(worldFolderPath).removeRecursively();
        }
    }
    loadFolders(MainWindow::folderSelected);

}

void MainWindow::showAboutDialog() {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("关于"));
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *label = new QLabel(dialog);
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    label->setText(tr("GitHub: <a href='https://github.com/Yamico/PalworldArchiveManager/issues'>https://github.com/Yamico/PalworldArchiveManager/issues</a> <br />欢迎提出issue!"));

    layout->addWidget(label);

    dialog->setLayout(layout);
    dialog->exec();
}

void MainWindow::showHelpDialog() {
    if (helpDialog) {
        helpDialog->raise();
        helpDialog->activateWindow();
        return;
    }
    helpDialog  = new QDialog(this);
    helpDialog->setWindowTitle(tr("回档方法"));
    QVBoxLayout *layout = new QVBoxLayout(helpDialog);

    QLabel *label = new QLabel(tr("1. 选择<打开存档目录>\n2. 在打开的页面中找到自己的SteamID，双击\n3. 在新页面中找到游戏目录(目录下有backup与Players文件夹)\n4. 看到两个目录时，当前所在目录即为正确目录")\
                               , helpDialog);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    layout->addWidget(label);

    helpDialog->setLayout(layout);
    helpDialog->setAttribute(Qt::WA_DeleteOnClose); 

    connect(helpDialog, &QDialog::destroyed, this, [this]() {
        this->helpDialog = nullptr;
    });
    helpDialog->show();
}

