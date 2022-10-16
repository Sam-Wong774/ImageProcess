#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ImageProcess.h"
#include "subMdiQLabel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImageProcessClass; };
class QMdiArea;
class QMdiSubWindow;
class QHBoxLayout;
class QProgressBar;
class QTime;
QT_END_NAMESPACE

class ImageProcess : public QMainWindow
{
    Q_OBJECT
public:
    ImageProcess(QWidget *parent = nullptr);
    ~ImageProcess();
private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionOpenFile_triggered();
    void updateMenus();
    subMdiQLabel* createSubMdiChild();
    void systemTimeUpdate();
    void qimageInfo();
    void on_actionClear_triggered();
    void about();
protected:
    void closeEvent(QCloseEvent* event)override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
private:
    int initMainFrame();
    void readSettings();
    void writeSettings();
    bool openQImage(const QString& fileName);
    bool loadQImage(const QString& fileName);
    bool loadQImage(const QImage& showQImage);
    subMdiQLabel* activeSubMdiChild() const;
    QMdiSubWindow* findMdiChild(const QString& fileName) const;
    // HWND restrictMouseActivity();
private:
    Ui::ImageProcessClass *ui;
    QMdiArea* pMdiArea;                         // ���Ĵ���
    QHBoxLayout* mStatusBarParaLayout;          // ״̬��������Ϣ����
    QTimer* mSystemTimer;                       // ϵͳʱ��
    QLabel* mTimerLable;                        // ��ʾʱ���ǩ
    QLabel* mQImageInfo;                        // ͼ����Ϣ
    QProgressBar* mProgressBar;                 // ͼ���������
    // subMdiQLabel* firstLabel;

    // ImageProcess �ۺ���
private slots:
    void on_actionColor2Gray_triggered();
    void on_actionR_Channel_triggered();
    void on_actionG_Channel_triggered();
    void on_actionB_Channel_triggered();
    // ImageEdit �ۺ���
private slots:

    // ImageFourierTransfrom �ۺ���
private slots:
    void on_actionDFT_triggered();
    void on_actionIDFT_triggered();
    void on_actionFFT_triggered();
    void on_actionIFFT_triggered();
    void on_actionFFTW_triggered();
    void on_actionIFFTW_triggered();

    // view �ۺ���
private slots:

};
