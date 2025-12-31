#include "ExtractDialog.h"
#include "ui_ExtractDialog.h"
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <thread>
#include <Archive.hpp>

ExtractDialog::ExtractDialog(const QString &archivePath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExtractDialog)
    , m_archivePath(archivePath)
{
    ui->setupUi(this);

    // Set archive path if provided
    if (!archivePath.isEmpty()) {
        ui->archiveLineEdit->setText(archivePath);
        ui->archiveLineEdit->setReadOnly(true);
        ui->archiveButton->setEnabled(false);
    }

    // Set default output directory
    ui->outputDirLineEdit->setText(QDir::currentPath() + "/extracted");

    setupConnections();
    validateInput();

    setStyleSheet(R"(
        ExtractDialog {
            background-color: #f5f5f5;
        }
        QLineEdit {
            padding: 6px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
    )");
}

ExtractDialog::~ExtractDialog()
{
    delete ui;
}

QString ExtractDialog::getArchivePath() const
{
    return ui->archiveLineEdit->text().trimmed();
}

QString ExtractDialog::getOutputDirectory() const
{
    return ui->outputDirLineEdit->text().trimmed();
}

bool ExtractDialog::shouldOverwrite() const
{
    return ui->overwriteCheckBox->isChecked();
}

bool ExtractDialog::shouldOpenFolder() const
{
    return ui->openFolderCheckBox->isChecked();
}

void ExtractDialog::setupConnections()
{
    connect(ui->archiveButton, &QPushButton::clicked, this, &ExtractDialog::onSelectArchive);
    connect(ui->outputDirButton, &QPushButton::clicked, this, &ExtractDialog::onSelectOutputDir);
    connect(ui->archiveLineEdit, &QLineEdit::textChanged, this, &ExtractDialog::validateInput);
    connect(ui->outputDirLineEdit, &QLineEdit::textChanged, this, &ExtractDialog::validateInput);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ExtractDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ExtractDialog::reject);
}

void ExtractDialog::onSelectArchive()
{
    QString filepath = QFileDialog::getOpenFileName(
        this,
        tr("Select Archive"),
        QString(),
        tr("VaultArchive Files (*.varc);;All Files (*)")
    );

    if (!filepath.isEmpty()) {
        ui->archiveLineEdit->setText(filepath);
        updateFileCount();
    }
}

void ExtractDialog::onSelectOutputDir()
{
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Directory"),
        ui->outputDirLineEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dirPath.isEmpty()) {
        ui->outputDirLineEdit->setText(dirPath);
    }
}

void ExtractDialog::validateInput()
{
    bool valid = !getArchivePath().isEmpty() && !getOutputDirectory().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void ExtractDialog::updateFileCount()
{
    QString archivePath = getArchivePath();
    if (archivePath.isEmpty()) {
        ui->fileCountLabel->setText(tr("No archive selected"));
        return;
    }

    VaultArchive::Archive archive;
    if (!archive.open(archivePath.toStdString())) {
        ui->fileCountLabel->setText(tr("Cannot open archive"));
        return;
    }

    uint64_t count = archive.getEntryCount();
    uint64_t totalSize = archive.getTotalOriginalSize();

    QString sizeStr;
    static const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(totalSize);

    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        ++unitIndex;
    }

    ui->fileCountLabel->setText(tr("%1 files (%2 %3)")
                                .arg(count)
                                .arg(size, 0, 'f', 2)
                                .arg(units[unitIndex]));
}
