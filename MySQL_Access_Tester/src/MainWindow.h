#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onTestClicked();

private:
    void logLine(const QString& s);
    void setStatusOk(const QString& s);
    void setStatusKo(const QString& s);

    Ui::MainWindow *ui;
};
