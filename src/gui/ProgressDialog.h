#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QString>

class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:
    ProgressDialog(const QString &title, const QString &label,
                   int minimum, int maximum, QWidget *parent = nullptr);
    ~ProgressDialog();

    void setCurrentFile(const QString &filename);
    void setStatus(const QString &status);

signals:
    void canceled();

private:
    QLabel *m_currentFileLabel;
    QProgressBar *m_progressBar;

    void setupUi(const QString &title, const QString &label);
};

#endif // PROGRESSDIALOG_H
