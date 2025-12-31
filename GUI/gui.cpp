#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QHeaderView>
#include <QInputDialog>
#include <QThread>
#include <QDebug>

#include <filesystem>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>


#include "../include/back_up/back_up.h"
#include<iostream>
namespace fs = std::filesystem;

class BackupGUI : public QMainWindow {
    Q_OBJECT

public:
    BackupGUI(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("数据备份软件");
        setMinimumSize(800, 600);

        // 创建主部件和布局
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        setCentralWidget(centralWidget);

        // 创建文件选择区域
        createFileSelectionArea(mainLayout);

        // 创建选项区域
        createOptionsArea(mainLayout);

        // 创建元数据显示区域
        createMetadataArea(mainLayout);

        // 创建进度条
        progressBar = new QProgressBar(this);
        progressBar->setVisible(false);
        mainLayout->addWidget(progressBar);

        // 创建按钮区域
        createButtonArea(mainLayout);

        // 初始化文件列表（空目录）
        updateFileList();
    }

private slots:
    // 选择备份目录
    void selectBackupDirectory() {
        QString dir = QFileDialog::getExistingDirectory(this, "选择备份目录");
        if (!dir.isEmpty()) {
            backupDirEdit->setText(dir);
            updateFileList(); // 选择目录后刷新文件列表
        }
    }

    // 选择保存位置
    void selectSaveLocation() {
        // 1. 定义过滤器：允许显示所有文件 + 筛选.backup文件（用户可切换）
        QString filter = "备份文件 (*.backup);;所有文件 (*.*)";
        // 2. 打开保存对话框（默认文件名可留空，也可设为"untitled.backup"）
        QString file = QFileDialog::getSaveFileName(
            this, 
            "保存备份文件", 
            "",  // 默认保存路径（可改为固定路径，如"/home/hzh/backup"）
            filter
        );
        if (!file.isEmpty()) {
 
            if (!file.endsWith(".backup", Qt::CaseInsensitive)) {
                file += ".backup";
            }
            saveLocationEdit->setText(file);
        }
    }

    // 更新文件列表
    void updateFileList() {
        fileList->clear(); // 清空现有列表
        std::string dirStr = backupDirEdit->text().toStdString();
        
        // 校验目录有效性
        if (dirStr.empty() || !fs::exists(fs::path(dirStr)) || !fs::is_directory(fs::path(dirStr))) {
            return;
        }

        // 前端实现：遍历目录下所有文件 + 筛选逻辑
        try {
            // 遍历目录下所有条目（不递归）
            for (const auto& entry : fs::directory_iterator(fs::path(dirStr))) {
                // 应用筛选规则：返回true表示保留，false表示排除
                if (filterFile(entry)) {
                    QListWidgetItem *item = new QListWidgetItem(
                        QString::fromStdString(entry.path().string())
                    );
                    item->setCheckState(Qt::Checked);
                    fileList->addItem(item);
                }
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "错误", QString("无法加载文件列表: %1").arg(e.what()));
        }
    }

    // 执行备份
    void performBackup() {
        if (fileList->count() == 0) 
        {
        QMessageBox::warning(this, "警告", "文件列表为空，请先选择备份目录");
        return;
        }
        if (saveLocationEdit->text().isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择备份文件保存位置");
            return;
        }

        // 1. 收集过滤掉的文件路径
        std::vector<std::string> filteredFiles;
        for (int i = 0; i < fileList->count(); ++i) {
            QListWidgetItem *item = fileList->item(i);
            if (item->checkState() == Qt::Unchecked) {
                filteredFiles.push_back(item->text().toStdString());
            }
        }
        if(filteredFiles.size()==fileList->count())
        {
            QMessageBox::warning(this, "警告", "没有选中的文件进行备份");
            return;
        }
        std::string dirStr = backupDirEdit->text().toStdString();
        std::string strFilteredFiles("");
        for(int i=0;i<filteredFiles.size();i++)
        {
            std::string relativePath = filteredFiles[i].substr(dirStr.size()+1);
            strFilteredFiles.append(std::format("{}\n",relativePath));
        }
        progressBar->setVisible(true);
        progressBar->setValue(0);
       
        std::string backupResult = data_packet::back_up(
            backupDirEdit->text().toStdString(),
            saveLocationEdit->text().toStdString(),
            huffmanRadio->isChecked()?"HUFFMAN":"LZ77",
            "AES_256_CBC",
            passwordEdit->text().toStdString(),
            strFilteredFiles
        );
        // 模拟进度
        
        for (int i = 0; i <= 100; i += 10) {
            progressBar->setValue(i);
            QApplication::processEvents(); // 刷新UI
            QThread::msleep(50); // 模拟耗时
        }

        progressBar->setVisible(false);
        if (backupResult == "OK") {
            QMessageBox::information(this, "成功", "备份完成");
        } else {
            QMessageBox::critical(this, "失败", QString("备份失败: %1").arg(backupResult.c_str()));
        }
    }

    // 加载备份文件
    void loadBackupFile() {
        QString file = QFileDialog::getOpenFileName(this, "打开备份文件", "", "备份文件 (*.backup)");
        if (file.isEmpty()) return;

        std::string infoStr = data_packet::info(file.toStdString());
        if (infoStr.find("Could not open output file") != std::string::npos) {
            QMessageBox::critical(this, "错误", "无法打开备份文件");
            return;
        }

        displayMetadata(infoStr);
        QMessageBox::information(this, "提示", "备份文件加载成功");
    }

    // 恢复备份（仅前端框架）
    void restoreBackup() {
        QString backupFile = QFileDialog::getOpenFileName(this, "选择备份文件", "", "备份文件 (*.backup)");
        if (backupFile.isEmpty()) return;

        QString restoreDir = QFileDialog::getExistingDirectory(this, "选择恢复目录");
        if (restoreDir.isEmpty()) return;
        
        // 获取备份文件信息
        std::string infoStr = data_packet::info(backupFile.toStdString());
        if (infoStr.find("Could not open output file") != std::string::npos) {
            QMessageBox::critical(this, "错误", "无法打开备份文件");
            return;
        }

        // 解析加密方式
        std::string password;
        bool isEncrypted = (infoStr.find("encryption method:AES 256 CBC") != std::string::npos);
        std::cout<<infoStr<<"\n";
        std::cout<<isEncrypted<<"\n";
        if (isEncrypted) {
            bool ok;
            QString qPassword = QInputDialog::getText(
                this, 
                "输入密码", 
                "请输入解密密码:", 
                QLineEdit::Password, 
                "", 
                &ok
            );
            if (!ok || qPassword.isEmpty()) {
                QMessageBox::warning(this, "取消", "恢复操作已取消");
                return;
            }
            password = qPassword.toStdString();
        }

        progressBar->setVisible(true);
        progressBar->setValue(0);

        std::string restoreResult=data_packet::restore_backup(
            backupFile.toStdString(),    // 备份文件路径
            restoreDir.toStdString(),    // 恢复目录
            password
        );
        // 模拟进度
        for (int i = 0; i <= 100; i += 10) {
            progressBar->setValue(i);
            QApplication::processEvents();
            QThread::msleep(50);
        }

        progressBar->setVisible(false);

        if (restoreResult == "OK") {
            QMessageBox::information(this, "成功", "恢复完成");
        } else {
            QMessageBox::critical(this, "失败", QString("恢复失败: %1").arg(restoreResult.c_str()));
        }
    }

private:
    // UI组件
    QListWidget *fileList;
    QLineEdit *backupDirEdit;
    QLineEdit *saveLocationEdit;
    QLineEdit *fileFilterEdit;
    QLineEdit *passwordEdit;
    QCheckBox *showHiddenCheckBox;
    QRadioButton *huffmanRadio;
    QRadioButton *lz77Radio;
    QTreeWidget *metadataTree;
    QProgressBar *progressBar;

    // 核心筛选逻辑（前端实现，返回true=保留该文件，false=排除）
    bool filterFile(const fs::directory_entry& entry) {

        std::string filename = entry.path().filename().string();

        // 1. 排除隐藏文件（若未勾选“显示隐藏文件”）
        if (!showHiddenCheckBox->isChecked() && !filename.empty() && filename.front() == '.') {
            return false;
        }

        // 2. 应用用户输入的筛选文本（包含匹配，不区分大小写）
        QString filterText = fileFilterEdit->text().trimmed(); // 去除首尾空格
        if (!filterText.isEmpty()) {
            QString qFilename = QString::fromStdString(filename);
            // 文件名不包含筛选文本 → 排除
            if (!qFilename.contains(filterText, Qt::CaseInsensitive)) {
                return false;
            }
        }

        // 所有条件都满足 → 保留该文件
        return true;
    }

    // 创建文件选择区域
    void createFileSelectionArea(QVBoxLayout *mainLayout) {
        QGroupBox *fileGroup = new QGroupBox("文件选择");
        QVBoxLayout *layout = new QVBoxLayout(fileGroup);

        // 备份目录选择
        QHBoxLayout *dirLayout = new QHBoxLayout();
        backupDirEdit = new QLineEdit();
        backupDirEdit->setPlaceholderText("请选择备份目录...");
        backupDirEdit->setStyleSheet("QLineEdit:read-only { background-color: #f0f0f0; }");
        backupDirEdit->setReadOnly(true);
        QPushButton *browseDirBtn = new QPushButton("浏览...");
        connect(browseDirBtn, &QPushButton::clicked, this, &BackupGUI::selectBackupDirectory);
        dirLayout->addWidget(new QLabel("备份目录:"));
        dirLayout->addWidget(backupDirEdit);
        dirLayout->addWidget(browseDirBtn);

        // 文件筛选
        QHBoxLayout *filterLayout = new QHBoxLayout();
        fileFilterEdit = new QLineEdit();
        fileFilterEdit->setPlaceholderText("输入文件名关键词筛选（支持模糊匹配）...");
        // 筛选文本变化时实时刷新列表
        connect(fileFilterEdit, &QLineEdit::textChanged, this, &BackupGUI::updateFileList);
        
        showHiddenCheckBox = new QCheckBox("显示隐藏文件");
        connect(showHiddenCheckBox, &QCheckBox::toggled, this, &BackupGUI::updateFileList);
        
        filterLayout->addWidget(new QLabel("筛选:"));
        filterLayout->addWidget(fileFilterEdit);
        filterLayout->addWidget(showHiddenCheckBox);

        // 文件列表
        fileList = new QListWidget();
        fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);

        layout->addLayout(dirLayout);
        layout->addLayout(filterLayout);
        layout->addWidget(new QLabel("文件列表:"));
        layout->addWidget(fileList);

        mainLayout->addWidget(fileGroup);
    }

    // 创建选项区域
    void createOptionsArea(QVBoxLayout *mainLayout) {
        QGroupBox *optionsGroup = new QGroupBox("备份选项");
        QVBoxLayout *layout = new QVBoxLayout(optionsGroup);

        // 保存位置
        QHBoxLayout *saveLayout = new QHBoxLayout();
        saveLocationEdit = new QLineEdit();
        saveLocationEdit->setPlaceholderText("请选择备份文件保存位置...");
        saveLocationEdit->setReadOnly(true); // 禁止手动输入，仅能通过浏览按钮赋值
        saveLocationEdit->setStyleSheet("QLineEdit:read-only { background-color: #f0f0f0; }");
        QPushButton *browseSaveBtn = new QPushButton("浏览...");
        connect(browseSaveBtn, &QPushButton::clicked, this, &BackupGUI::selectSaveLocation);
        saveLayout->addWidget(new QLabel("保存位置:"));
        
        saveLayout->addWidget(saveLocationEdit);
        saveLayout->addWidget(browseSaveBtn);

        // 压缩算法选择
        QHBoxLayout *compressionLayout = new QHBoxLayout();
        huffmanRadio = new QRadioButton("Huffman压缩");
        lz77Radio = new QRadioButton("LZ77压缩");
        huffmanRadio->setChecked(true); // 默认选择Huffman
        compressionLayout->addWidget(new QLabel("压缩算法:"));
        compressionLayout->addWidget(huffmanRadio);
        compressionLayout->addWidget(lz77Radio);
        compressionLayout->addStretch();

        // 加密选项
        QHBoxLayout *encryptionLayout = new QHBoxLayout();
        passwordEdit = new QLineEdit();
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setPlaceholderText("输入加密密码（为空则使用默认加密）");
        encryptionLayout->addWidget(new QLabel("加密密码:"));
        encryptionLayout->addWidget(passwordEdit);

        layout->addLayout(saveLayout);
        layout->addLayout(compressionLayout);
        layout->addLayout(encryptionLayout);

        mainLayout->addWidget(optionsGroup);
    }

    // 创建元数据显示区域
    void createMetadataArea(QVBoxLayout *mainLayout) {
        QGroupBox *metadataGroup = new QGroupBox("文件元数据");
        QVBoxLayout *layout = new QVBoxLayout(metadataGroup);

        metadataTree = new QTreeWidget();
        metadataTree->setColumnCount(2);
        metadataTree->setHeaderLabels({"元数据信息", "值"});

        metadataTree->setColumnWidth(0, 300); // 初始宽度150
        metadataTree->header()->setSectionResizeMode(0, QHeaderView::Interactive); // 可手动拖+自适应
        metadataTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
       
        layout->addWidget(metadataTree);

        mainLayout->addWidget(metadataGroup);
    }

    // 创建按钮区域
    void createButtonArea(QVBoxLayout *mainLayout) {
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        QPushButton *backupBtn = new QPushButton("执行备份");
        QPushButton *loadBtn = new QPushButton("加载备份文件");
        QPushButton *restoreBtn = new QPushButton("恢复备份");
        QPushButton *exitBtn = new QPushButton("退出");

        connect(backupBtn, &QPushButton::clicked, this, &BackupGUI::performBackup);
        connect(loadBtn, &QPushButton::clicked, this, &BackupGUI::loadBackupFile);
        connect(restoreBtn, &QPushButton::clicked, this, &BackupGUI::restoreBackup);
        connect(exitBtn, &QPushButton::clicked, qApp, &QApplication::quit);

        buttonLayout->addWidget(backupBtn);
        buttonLayout->addWidget(loadBtn);
        buttonLayout->addWidget(restoreBtn);
        buttonLayout->addWidget(exitBtn);

        mainLayout->addLayout(buttonLayout);
    }

    // 显示元数据
void displayMetadata(const std::string& metadataStr) 
{
    metadataTree->clear(); // 清空原有内容

    // 1. 分割后端返回的元数据字符串为单行（按换行符分割）
    std::vector<std::string> lines;
    size_t pos = 0;
    size_t nextPos;
    while ((nextPos = metadataStr.find('\n', pos)) != std::string::npos) {
        std::string line = metadataStr.substr(pos, nextPos - pos);
        if (!line.empty()) lines.push_back(line); // 跳过空行
        pos = nextPos + 1;
    }
    // 处理最后一行（无换行符的情况）
    if (pos < metadataStr.size()) {
        std::string lastLine = metadataStr.substr(pos);
        if (!lastLine.empty()) lines.push_back(lastLine);
    }

    // 2. 定义元数据分类节点（仅展示后端返回的项，无则不显示对应节点）
    QTreeWidgetItem *basicItem = new QTreeWidgetItem({"基本信息"});
    QTreeWidgetItem *compressionItem = new QTreeWidgetItem({"压缩信息"});
    QTreeWidgetItem *encryptionItem = new QTreeWidgetItem({"加密信息"});
    QTreeWidgetItem *fileListItem = new QTreeWidgetItem({"包含文件"});
    
    bool isFileListSection = false; // 标记是否进入“包含文件”段落

    // 3. 逐行解析后端返回的元数据
    for (const auto& line : lines) {
        // 检测是否进入“包含文件”区域（后端若返回"all file names:"则开始列文件）
        if (line == "all file names:") {
            isFileListSection = true;
            continue;
        }

        // 处理“包含文件”列表
        if (isFileListSection) {
            fileListItem->addChild(new QTreeWidgetItem({QString::fromStdString(line), ""}));
            continue;
        }

        // 解析键值对（格式：key: value）
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue; // 非键值对格式，跳过

        std::string key = line.substr(0, colonPos);
        std::string value = (colonPos + 1 <= line.size()) ? line.substr(colonPos + 1) : "";
        QString qKey = QString::fromStdString(key);
        QString qValue = QString::fromStdString(value);

        // 按后端返回的键分类添加（仅添加后端存在的项，无则不显示）
        if (key == "version" || key == "file size" || key == "original file size" || key == "create time") {
            basicItem->addChild(new QTreeWidgetItem({qKey, qValue}));
        } else if (key == "compression method") {
            compressionItem->addChild(new QTreeWidgetItem({qKey, qValue}));
        } else if (key == "encryption method") {
            encryptionItem->addChild(new QTreeWidgetItem({qKey, qValue}));
        }
    }

    // 4. 仅添加有内容的节点（后端无对应项则不显示该分类）
    if (basicItem->childCount() > 0) {
        metadataTree->addTopLevelItem(basicItem);
    }
    if (compressionItem->childCount() > 0) {
        metadataTree->addTopLevelItem(compressionItem);
    }
    if (encryptionItem->childCount() > 0) {
        metadataTree->addTopLevelItem(encryptionItem);
    }
    if (fileListItem->childCount() > 0) {
        metadataTree->addTopLevelItem(fileListItem);
    }

    // 5. 展开所有有内容的节点（方便查看）
    metadataTree->expandAll();
}
};
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qSetMessagePattern("%{time} %{message}"); // 可选：添加时间戳
    qInstallMessageHandler(nullptr); // 禁用Qt默认日志重定向，输出到stdout
    BackupGUI window;
    window.show();

    return app.exec();
}

// 必须添加：MOC生成的元对象代码（文件名需与cpp一致）
#include "gui.moc"