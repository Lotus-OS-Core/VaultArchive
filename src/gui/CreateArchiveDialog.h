#ifndef CREATEARCHIVEDIALOG_H
#define CREATEARCHIVEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <Archive.hpp>

namespace Ui {
class CreateArchiveDialog;
}

class CreateArchiveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateArchiveDialog(QWidget *parent = nullptr);
    ~CreateArchiveDialog();

private slots:
    void onSelectOutputFile();
    void onAddFiles();
    void onAddDirectory();
    void onRemoveSelected();
    void onClearAll();
    void onAccept();
    void updateFileList();

private:
    Ui::CreateArchiveDialog *ui;
    QStringList m_files;

    void setupConnections();
    void updateStatus();
    QString calculateTotalSize() const;
};

#endif // CREATEARCHIVEDIALOG_H
