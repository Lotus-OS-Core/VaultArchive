#ifndef EXTRACTDIALOG_H
#define EXTRACTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

namespace Ui {
class ExtractDialog;
}

class ExtractDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtractDialog(const QString &archivePath = QString(), QWidget *parent = nullptr);
    ~ExtractDialog();

    QString getArchivePath() const;
    QString getOutputDirectory() const;
    bool shouldOverwrite() const;
    bool shouldOpenFolder() const;

private slots:
    void onSelectArchive();
    void onSelectOutputDir();
    void validateInput();

private:
    Ui::ExtractDialog *ui;
    QString m_archivePath;

    void setupConnections();
    void updateFileCount();
};

#endif // EXTRACTDIALOG_H
