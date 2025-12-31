#include "CreateArchiveDialog.h"
#include "ui_CreateArchiveDialog.h"
#include <QFileDialog>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <thread>
#include <chrono>

CreateArchiveDialog::CreateArchiveDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateArchiveDialog)
{
    ui->setupUi(this);

    setupConnections();
    updateStatus();

    setStyleSheet(R"(
        CreateArchiveDialog {
            background-color: #f5f5f5;
        }
        QListWidget {
            background-color: white;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        QLineEdit {
            padding: 6px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
        QCheckBox {
            spacing: 8px;
        }
    )");
}

CreateArchiveDialog::~CreateArchiveDialog()
{
    delete ui;
}

void CreateArchiveDialog::setupConnections()
{
    connect(ui->outputFileButton, &QPushButton::clicked, this, &CreateArchiveDialog::onSelectOutputFile);
    connect(ui->addFilesButton, &QPushButton::clicked, this, &CreateArchiveDialog::onAddFiles);
    connect(ui->addDirectoryButton, &QPushButton::clicked, this, &CreateArchiveDialog::onAddDirectory);
    connect(ui->removeButton, &QPushButton::clicked, this, &CreateArchiveDialog::onRemoveSelected);
    connect(ui->clearButton, &QPushButton::clicked, this, &CreateArchiveDialog::onClearAll);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CreateArchiveDialog::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CreateArchiveDialog::onSelectOutputFile()
{
    QString filepath = QFileDialog::getSaveFileName(
        this,
        tr("Create Archive"),
        QString("archive.varc"),
        tr("VaultArchive Files (*.varc);;All Files (*)")
    );

    if (!filepath.isEmpty()) {
        // Ensure .varc extension
        if (!filepath.endsWith(".varc", Qt::CaseInsensitive)) {
            filepath += ".varc";
        }
        ui->outputFileLineEdit->setText(filepath);
    }
}

void CreateArchiveDialog::onAddFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Files to Archive"),
        QString(),
        tr("All Files (*)")
    );

    if (!files.isEmpty()) {
        m_files.append(files);
        updateFileList();
    }
}

void CreateArchiveDialog::onAddDirectory()
{
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select Directory to Archive"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dirPath.isEmpty()) {
        m_files.append(dirPath);
        updateFileList();
    }
}

void CreateArchiveDialog::onRemoveSelected()
{
    QList<QListWidgetItem*> selected = ui->fileListWidget->selectedItems();
    for (auto item : selected) {
        int row = ui->fileListWidget->row(item);
        if (row >= 0 && row < m_files.size()) {
            m_files.removeAt(row);
        }
        delete item;
    }
    updateStatus();
}

void CreateArchiveDialog::onClearAll()
{
    m_files.clear();
    ui->fileListWidget->clear();
    updateStatus();
}

void CreateArchiveDialog::onAccept()
{
    QString outputFile = ui->outputFileLineEdit->text().trimmed();

    if (outputFile.isEmpty()) {
        QMessageBox::warning(this, tr("No Output File"),
                            tr("Please select an output file for the archive."));
        return;
    }

    if (m_files.isEmpty()) {
        QMessageBox::warning(this, tr("No Files Selected"),
                            tr("Please add at least one file or directory."));
        return;
    }

    // Check if file exists
    if (QFile::exists(outputFile)) {
        if (QMessageBox::question(this, tr("File Exists"),
                                 tr("The file %1 already exists. Overwrite?").arg(outputFile))
            != QMessageBox::Yes) {
            return;
        }
    }

    // Get options
    VaultArchive::CreateOptions options;
    options.compress = ui->compressCheckBox->isChecked();
    options.compressionLevel = ui->compressionLevelCombo->currentIndex();

    if (ui->encryptCheckBox->isChecked()) {
        bool ok;
        QString password = QInputDialog::getText(this, tr("Encryption Password"),
            tr("Enter password for encryption:"), QLineEdit::Password, QString(), &ok);

        if (!ok || password.isEmpty()) {
            QMessageBox::warning(this, tr("No Password"),
                                tr("Encryption requires a password."));
            return;
        }

        QString confirm = QInputDialog::getText(this, tr("Confirm Password"),
            tr("Confirm password:"), QLineEdit::Password, QString(), &ok);

        if (!ok || password != confirm) {
            QMessageBox::warning(this, tr("Password Mismatch"),
                                tr("Passwords do not match."));
            return;
        }

        options.encrypt = true;
        options.password = password.toStdString();
    }

    // Create archive
    VaultArchive::Archive archive;
    if (!archive.create(outputFile.toStdString())) {
        QMessageBox::critical(this, tr("Error"),
                            tr("Failed to create archive: %1").arg(archive.getLastError().c_str()));
        return;
    }

    // Add files
    QEventLoop loop;
    bool completed = false;
    QString errorMessage;

    std::thread worker([this, &archive, &options, &completed, &errorMessage, &loop]() {
        try {
            auto result = archive.addFiles(m_files.toStdVector(), options);
            if (result.success) {
                archive.save();
                completed = true;
            } else {
                errorMessage = QString::fromStdString(archive.getLastError());
            }
        } catch (const std::exception& e) {
            errorMessage = e.what();
        }
        loop.quit();
    });

    // Show progress
    QProgressDialog progress(tr("Creating archive..."), tr("Cancel"), 0, 0, this);
    progress.setModal(true);
    connect(&progress, &QProgressDialog::canceled, &loop, &QEventLoop::quit);
    progress.show();
    QApplication::processEvents();

    loop.exec();
    worker.join();

    if (completed) {
        QMessageBox::information(this, tr("Success"),
                                tr("Archive created successfully:\n%1").arg(outputFile));
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"),
                            tr("Failed to create archive: %1").arg(errorMessage));
    }
}

void CreateArchiveDialog::updateFileList()
{
    ui->fileListWidget->clear();

    for (const QString &file : m_files) {
        QFileInfo info(file);
        QString displayName = info.isDir() ? file + "/" : file;
        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setIcon(info.isDir() ?
                      QIcon::fromTheme("folder") :
                      QIcon::fromTheme("text-x-generic"));
        ui->fileListWidget->addItem(item);
    }

    updateStatus();
}

void CreateArchiveDialog::updateStatus()
{
    int count = m_files.size();
    ui->fileCountLabel->setText(tr("%1 item(s)").arg(count));

    QString totalSize = calculateTotalSize();
    ui->totalSizeLabel->setText(tr("Total: %1").arg(totalSize));

    ui->removeButton->setEnabled(count > 0);
    ui->clearButton->setEnabled(count > 0);
}

QString CreateArchiveDialog::calculateTotalSize() const
{
    quint64 totalBytes = 0;

    for (const QString &path : m_files) {
        QFileInfo info(path);

        if (info.isDir()) {
            // Calculate directory size recursively
            QDir dir(path);
            QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
            QFileInfoList entries = dir.entryInfoList(filters, QDir::Recursive);

            for (const QFileInfo &entry : entries) {
                if (entry.isFile()) {
                    totalBytes += entry.size();
                }
            }
        } else {
            totalBytes += info.size();
        }
    }

    // Format size
    static const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(totalBytes);

    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        ++unitIndex;
    }

    return QString::number(size, 'f', 2) + " " + units[unitIndex];
}
