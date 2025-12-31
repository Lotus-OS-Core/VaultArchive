#include "MainWindow.h"
#include "AboutDialog.h"
#include "CreateArchiveDialog.h"
#include "ExtractDialog.h"
#include "ProgressDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <thread>
#include <chrono>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_archive(std::make_unique<VaultArchive::Archive>())
    , m_currentArchivePath()
    , m_modified(false)
    , m_tableWidget(new QTableWidget(this))
    , m_statusBar(new QStatusBar(this))
    , m_toolBar(new QToolBar(this))
    , m_statusLabel(new QLabel("Ready", this))
    , m_fileCountLabel(new QLabel("0 files", this))
    , m_progressBar(new QProgressBar(this))
    , m_progressLabel(new QLabel("", this))
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    createConnections();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // Set window properties
    resize(900, 600);
    setMinimumSize(600, 400);

    // Central widget
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Table widget setup
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Size" << "Type" << "Date" << "Checksum");
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Connect context menu
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::onContextMenuRequested);

    mainLayout->addWidget(m_tableWidget);

    // Set central widget
    setCentralWidget(centralWidget);

    // Set status bar
    setStatusBar(m_statusBar);
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    m_actionNewArchive = new QAction(QIcon::fromTheme("document-new"), tr("&New Archive"), this);
    m_actionNewArchive->setShortcut(QKeySequence::New);
    fileMenu->addAction(m_actionNewArchive);

    m_actionOpenArchive = new QAction(QIcon::fromTheme("document-open"), tr("&Open Archive"), this);
    m_actionOpenArchive->setShortcut(QKeySequence::Open);
    fileMenu->addAction(m_actionOpenArchive);

    m_actionSaveArchive = new QAction(QIcon::fromTheme("document-save"), tr("&Save Archive"), this);
    m_actionSaveArchive->setShortcut(QKeySequence::Save);
    m_actionSaveArchive->setEnabled(false);
    fileMenu->addAction(m_actionSaveArchive);

    fileMenu->addSeparator();

    m_actionExit = new QAction(QIcon::fromTheme("application-exit"), tr("E&xit"), this);
    m_actionExit->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(m_actionExit);

    // Actions menu
    QMenu *actionsMenu = menuBar()->addMenu(tr("&Actions"));

    m_actionAddFiles = new QAction(QIcon::fromTheme("list-add"), tr("&Add Files"), this);
    m_actionAddFiles->setEnabled(false);
    actionsMenu->addAction(m_actionAddFiles);

    m_actionExtract = new QAction(QIcon::fromTheme("document-save-as"), tr("&Extract"), this);
    m_actionExtract->setEnabled(false);
    actionsMenu->addAction(m_actionExtract);

    actionsMenu->addSeparator();

    m_actionVerify = new QAction(QIcon::fromTheme("dialog-ok"), tr("&Verify"), this);
    m_actionVerify->setEnabled(false);
    actionsMenu->addAction(m_actionVerify);

    actionsMenu->addSeparator();

    m_actionList = new QAction(tr("&List Contents"), this);
    actionsMenu->addAction(m_actionList);

    m_actionLock = new QAction(QIcon::fromTheme("lock"), tr("&Lock Archive"), this);
    m_actionLock->setEnabled(false);
    actionsMenu->addAction(m_actionLock);

    m_actionUnlock = new QAction(QIcon::fromTheme("unlock"), tr("&Unlock Archive"), this);
    m_actionUnlock->setEnabled(false);
    actionsMenu->addAction(m_actionUnlock);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    m_actionHelp = new QAction(QIcon::fromTheme("help-contents"), tr("&Contents"), this);
    m_actionHelp->setShortcut(QKeySequence::HelpContents);
    helpMenu->addAction(m_actionHelp);

    m_actionAbout = new QAction(QIcon::fromTheme("help-about"), tr("&About"), this);
    helpMenu->addAction(m_actionAbout);
}

void MainWindow::setupToolBar()
{
    m_toolBar->setMovable(false);
    m_toolBar->setFloatable(false);
    m_toolBar->addAction(m_actionNewArchive);
    m_toolBar->addAction(m_actionOpenArchive);
    m_toolBar->addAction(m_actionSaveArchive);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_actionAddFiles);
    m_toolBar->addAction(m_actionExtract);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_actionVerify);
    addToolBar(m_toolBar);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel->setMinimumWidth(150);
    m_fileCountLabel->setMinimumWidth(100);
    m_progressLabel->setMinimumWidth(200);
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);

    m_statusBar->addPermanentWidget(m_statusLabel);
    m_statusBar->addPermanentWidget(m_fileCountLabel);
    m_statusBar->addPermanentWidget(m_progressLabel);
    m_statusBar->addPermanentWidget(m_progressBar);
}

void MainWindow::createConnections()
{
    connect(m_actionNewArchive, &QAction::triggered, this, &MainWindow::onActionNewArchive);
    connect(m_actionOpenArchive, &QAction::triggered, this, &MainWindow::onActionOpenArchive);
    connect(m_actionSaveArchive, &QAction::triggered, this, &MainWindow::onActionSaveArchive);
    connect(m_actionAddFiles, &QAction::triggered, this, &MainWindow::onActionAddFiles);
    connect(m_actionExtract, &QAction::triggered, this, &MainWindow::onActionExtract);
    connect(m_actionVerify, &QAction::triggered, this, &MainWindow::onActionVerify);
    connect(m_actionList, &QAction::triggered, this, &MainWindow::onActionList);
    connect(m_actionLock, &QAction::triggered, this, &MainWindow::onActionLock);
    connect(m_actionUnlock, &QAction::triggered, this, &MainWindow::onActionUnlock);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(m_actionHelp, &QAction::triggered, this, &MainWindow::onActionHelp);
    connect(m_actionExit, &QAction::triggered, this, &MainWindow::onActionExit);

    connect(this, &MainWindow::archiveLoaded, this, &MainWindow::onArchiveLoaded);
    connect(this, &MainWindow::statusMessage, this, &MainWindow::onStatusUpdate);
    connect(this, &MainWindow::progressUpdate, this, &MainWindow::onProgressUpdate);
}

void MainWindow::onActionNewArchive()
{
    CreateArchiveDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        emit statusMessage(tr("Archive created successfully"));
    }
}

void MainWindow::onActionOpenArchive()
{
    QString filepath = QFileDialog::getOpenFileName(
        this,
        tr("Open Archive"),
        QString(),
        tr("VaultArchive Files (*.varc);;All Files (*)")
    );

    if (!filepath.isEmpty()) {
        if (loadArchive(filepath)) {
            emit archiveLoaded(filepath);
        }
    }
}

void MainWindow::onActionSaveArchive()
{
    if (m_archive->isModified()) {
        if (m_archive->save()) {
            m_modified = false;
            emit statusMessage(tr("Archive saved successfully"));
        } else {
            showError(tr("Save Error"), m_archive->getLastError());
        }
    }
}

void MainWindow::onActionAddFiles()
{
    if (!m_archive->isOpen()) {
        showError(tr("No Archive"), tr("Please open an archive first"));
        return;
    }

    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Add Files to Archive"),
        QString(),
        tr("All Files (*)")
    );

    if (!files.isEmpty()) {
        VaultArchive::CreateOptions options;
        options.compress = true;
        options.compressionLevel = 6;

        ProgressDialog progress(tr("Adding Files"), tr("Adding files..."), 0, 100, this);
        progress.setModal(true);

        QEventLoop loop;
        connect(&progress, &ProgressDialog::canceled, &loop, &QEventLoop::quit);

        std::thread worker([this, &files, &options, &progress]() {
            auto result = m_archive->addFiles(files.toStdVector(), options);
            emit progressUpdate(100, tr("Complete"));
        });

        progress.exec();
        worker.join();

        m_modified = true;
        updateArchiveTable();
        updateWindowTitle(m_currentArchivePath);
        emit statusMessage(tr("Files added successfully"));
    }
}

void MainWindow::onActionExtract()
{
    if (!m_archive->isOpen()) {
        showError(tr("No Archive"), tr("Please open an archive first"));
        return;
    }

    ExtractDialog dialog(m_currentArchivePath, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit statusMessage(tr("Extraction complete"));
    }
}

void MainWindow::onActionVerify()
{
    if (!m_archive->isOpen()) {
        showError(tr("No Archive"), tr("Please open an archive first"));
        return;
    }

    emit statusMessage(tr("Verifying archive integrity..."));

    if (m_archive->verify()) {
        showInfo(tr("Verification"), tr("Archive integrity verified successfully"));
    } else {
        showError(tr("Verification Failed"), m_archive->getLastError());
    }

    emit statusMessage(tr("Verification complete"));
}

void MainWindow::onActionList()
{
    if (!m_archive->isOpen()) {
        showError(tr("No Archive"), tr("Please open an archive first"));
        return;
    }

    QString listing = m_archive->list();
    QMessageBox::information(this, tr("Archive Contents"), listing,
                            QMessageBox::Ok, QMessageBox::Ok);
}

void MainWindow::onActionLock()
{
    if (!m_archive->isOpen()) {
        return;
    }

    bool ok;
    QString password = QInputDialog::getText(this, tr("Lock Archive"),
        tr("Enter password to lock the archive:"), QLineEdit::Password,
        QString(), &ok);

    if (ok && !password.isEmpty()) {
        QString confirm = QInputDialog::getText(this, tr("Confirm Password"),
            tr("Confirm password:"), QLineEdit::Password, QString(), &ok);

        if (ok && password == confirm) {
            if (m_archive->lock(password.toStdString())) {
                m_modified = true;
                showInfo(tr("Success"), tr("Archive locked successfully"));
            } else {
                showError(tr("Error"), m_archive->getLastError());
            }
        } else if (ok) {
            showError(tr("Error"), tr("Passwords do not match"));
        }
    }
}

void MainWindow::onActionUnlock()
{
    if (!m_archive->isOpen()) {
        return;
    }

    QString password = QInputDialog::getText(this, tr("Unlock Archive"),
        tr("Enter password to unlock the archive:"), QLineEdit::Password,
        QString(), nullptr);

    if (!password.isEmpty()) {
        if (m_archive->unlock(password.toStdString())) {
            m_modified = true;
            updateArchiveTable();
            showInfo(tr("Success"), tr("Archive unlocked successfully"));
        } else {
            showError(tr("Error"), m_archive->getLastError());
        }
    }
}

void MainWindow::onActionAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onActionHelp()
{
    QMessageBox::information(this, tr("VaultArchive Help"),
        tr("VaultArchive is a secure file archiving utility.\n\n"
           "Use the File menu to create or open archives.\n"
           "Use the Actions menu to add files, extract, or verify archives.\n\n"
           "For command-line usage, run: varc --help\n\n"
           "See the man page for detailed documentation: man varc"),
        QMessageBox::Ok);
}

void MainWindow::onActionExit()
{
    if (m_modified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Save Changes"),
            tr("The archive has been modified. Do you want to save before exiting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            if (!m_archive->save()) {
                showError(tr("Save Error"), m_archive->getLastError());
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    close();
}

void MainWindow::onCellActivated(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    // Could implement preview or other actions on cell activation
}

void MainWindow::onArchiveLoaded(const QString &filepath)
{
    updateArchiveTable();
    updateWindowTitle(filepath);
}

void MainWindow::onStatusUpdate(const QString &message)
{
    m_statusLabel->setText(message);
}

void MainWindow::onProgressUpdate(int percent, const QString &currentFile)
{
    m_progressBar->setValue(percent);
    m_progressLabel->setText(currentFile);
    if (percent >= 100) {
        QTimer::singleShot(1000, this, [this]() {
            m_progressBar->setVisible(false);
            m_progressLabel->setText("");
        });
    } else {
        m_progressBar->setVisible(true);
    }
}

void MainWindow::updateArchiveTable()
{
    m_tableWidget->setRowCount(0);

    if (!m_archive->isOpen()) {
        m_fileCountLabel->setText("0 files");
        m_actionAddFiles->setEnabled(false);
        m_actionExtract->setEnabled(false);
        m_actionVerify->setEnabled(false);
        m_actionLock->setEnabled(false);
        m_actionUnlock->setEnabled(false);
        m_actionSaveArchive->setEnabled(false);
        return;
    }

    const auto &entries = m_archive->getEntries();
    m_tableWidget->setRowCount(static_cast<int>(entries.size()));

    for (size_t i = 0; i < entries.size(); ++i) {
        const auto &entry = entries[i];

        // Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(entry.getPath());
        nameItem->setData(Qt::UserRole, QString::fromStdString(entry.getPath()));
        m_tableWidget->setItem(static_cast<int>(i), 0, nameItem);

        // Size
        QString sizeStr = entry.getSizeString();
        if (entry.isCompressed() && entry.getCompressedSize() != entry.getOriginalSize()) {
            sizeStr += QString(" (%1)").arg(entry.getCompressedSizeString());
        }
        QTableWidgetItem *sizeItem = new QTableWidgetItem(sizeStr);
        m_tableWidget->setItem(static_cast<int>(i), 1, sizeItem);

        // Type
        QTableWidgetItem *typeItem = new QTableWidgetItem(entry.getTypeString());
        m_tableWidget->setItem(static_cast<int>(i), 2, typeItem);

        // Date
        auto time = entry.getModificationTime();
        auto tt = std::chrono::system_clock::to_time_t(time);
        QString dateStr = QDateTime::fromSecsSinceEpoch(tt).toString(Qt::SystemLocaleShortDate);
        QTableWidgetItem *dateItem = new QTableWidgetItem(dateStr);
        m_tableWidget->setItem(static_cast<int>(i), 3, dateItem);

        // Checksum (first 8 characters for brevity)
        QString checksum = QString::fromStdString(
            VaultArchive::CryptoEngine::bytesToHex(entry.getChecksum())).left(16);
        QTableWidgetItem *checksumItem = new QTableWidgetItem(checksum);
        m_tableWidget->setItem(static_cast<int>(i), 4, checksumItem);
    }

    m_tableWidget->resizeColumnsToContents();

    // Update actions
    m_fileCountLabel->setText(QString("%1 files").arg(entries.size()));
    m_actionAddFiles->setEnabled(true);
    m_actionExtract->setEnabled(true);
    m_actionVerify->setEnabled(true);
    m_actionLock->setEnabled(!m_archive->getHeader().isEncrypted());
    m_actionUnlock->setEnabled(m_archive->getHeader().isEncrypted());
    m_actionSaveArchive->setEnabled(false);  // Archives are auto-saved when modified
}

void MainWindow::updateWindowTitle(const QString &filepath)
{
    if (filepath.isEmpty()) {
        setWindowTitle(tr("VaultArchive GUI - v0.3.27"));
    } else {
        QFileInfo info(filepath);
        setWindowTitle(tr("%1 - VaultArchive GUI - v0.3.27").arg(info.fileName()));
    }
}

void MainWindow::clearArchive()
{
    m_archive->close();
    m_currentArchivePath.clear();
    m_modified = false;
    updateArchiveTable();
    updateWindowTitle();
}

bool MainWindow::loadArchive(const QString &filepath)
{
    QString password;

    // Check if archive might be encrypted
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        showError(tr("Open Error"), tr("Cannot open file: %1").arg(filepath));
        return false;
    }

    // Read first few bytes to check for encryption flag
    QByteArray header = file.read(64);
    file.close();

    // If encrypted flag is set, ask for password
    if (header.size() >= 8) {
        uint16_t flags = (static_cast<uint8_t>(header[6]) << 8) | static_cast<uint8_t>(header[7]);
        if (flags & 0x0001) {  // ENCRYPTED flag
            password = QInputDialog::getText(this, tr("Password Required"),
                tr("Enter password for archive:"), QLineEdit::Password, QString(), nullptr);
        }
    }

    if (!m_archive->open(filepath.toStdString(), password.toStdString())) {
        showError(tr("Open Error"), m_archive->getLastError());
        return false;
    }

    m_currentArchivePath = filepath;
    m_modified = false;
    return true;
}

void MainWindow::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(this, title, message, QMessageBox::Ok);
}

void MainWindow::showInfo(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message, QMessageBox::Ok);
}

bool MainWindow::confirmAction(const QString &title, const QString &message)
{
    return QMessageBox::question(this, title, message,
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::onContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos)
    // Could add context menu for selected items
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_modified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Save Changes"),
            tr("The archive has been modified. Do you want to save before exiting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            if (!m_archive->save()) {
                showError(tr("Save Error"), m_archive->getLastError());
                event->ignore();
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    event->accept();
}
