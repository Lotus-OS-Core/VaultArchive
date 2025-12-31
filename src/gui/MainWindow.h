#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QProgressBar>
#include <memory>
#include "Archive.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onActionNewArchive();
    void onActionOpenArchive();
    void onActionSaveArchive();
    void onActionAddFiles();
    void onActionExtract();
    void onActionVerify();
    void onActionList();
    void onActionLock();
    void onActionUnlock();
    void onActionAbout();
    void onActionHelp();
    void onActionExit();
    void onCellActivated(int row, int column);
    void onArchiveLoaded(const QString &filepath);
    void onStatusUpdate(const QString &message);
    void onProgressUpdate(int percent, const QString &currentFile);

signals:
    void archiveLoaded(const QString &filepath);
    void statusMessage(const QString &message);
    void progressUpdate(int percent, const QString &currentFile);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void createConnections();

    void updateArchiveTable();
    void updateWindowTitle(const QString &filepath = QString());
    void clearArchive();
    bool loadArchive(const QString &filepath);
    void showError(const QString &title, const QString &message);
    void showInfo(const QString &title, const QString &message);
    bool confirmAction(const QString &title, const QString &message);

    std::unique_ptr<VaultArchive::Archive> m_archive;
    QString m_currentArchivePath;
    bool m_modified;

    // UI elements
    QTableWidget *m_tableWidget;
    QStatusBar *m_statusBar;
    QToolBar *m_toolBar;
    QLabel *m_statusLabel;
    QLabel *m_fileCountLabel;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;

    // Actions
    QAction *m_actionNewArchive;
    QAction *m_actionOpenArchive;
    QAction *m_actionSaveArchive;
    QAction *m_actionAddFiles;
    QAction *m_actionExtract;
    QAction *m_actionVerify;
    QAction *m_actionList;
    QAction *m_actionLock;
    QAction *m_actionUnlock;
    QAction *m_actionAbout;
    QAction *m_actionHelp;
    QAction *m_actionExit;
};

#endif // MAINWINDOW_H
