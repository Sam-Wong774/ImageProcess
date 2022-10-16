#include "stdafx.h"
#include "ImageProcess.h"
#include <qscreen.h>
#include <qdatetime.h>
#include "myDFT.h"

ImageProcess::ImageProcess(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageProcessClass())
{
    ui->setupUi(this);   
    QCoreApplication::setOrganizationName("Provincial Laboratory of Optical Information and Pattern Recognition");
    QCoreApplication::setApplicationName("Image Process Software");

    pMdiArea = new QMdiArea;
    mStatusBarParaLayout = new QHBoxLayout(this->statusBar());
    mSystemTimer = new QTimer(this);
    mTimerLable = new QLabel();
    mQImageInfo = new QLabel();
    mProgressBar = new QProgressBar();
    mSystemTimer->start(1000);
    updateMenus();
    readSettings();
    initMainFrame();

    setUnifiedTitleAndToolBarOnMac(TRUE);
}

ImageProcess::~ImageProcess()
{
    delete mProgressBar;
    delete mQImageInfo;
    delete mTimerLable;
    delete mSystemTimer;
    delete mStatusBarParaLayout;
    delete pMdiArea;
    delete ui;
}

void ImageProcess::on_actionNew_triggered()
{
    subMdiQLabel* child = createSubMdiChild();
    child->newFile();
    pMdiArea->addSubWindow(child);
    child->show();
}

void ImageProcess::on_actionOpen_triggered()
{
    static const QString strImageFilter("(*.bmp);;(*.png);;(*.jpg);;(*.tif)");
    const QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), QDir::currentPath(), strImageFilter);
    if (!fileName.isEmpty())
        openQImage(fileName);
}

void ImageProcess::on_actionSave_triggered()
{
    if (activeSubMdiChild() && activeSubMdiChild()->save()) {
        statusBar()->showMessage(tr("Image Saved"), 2000);
    }
}

void ImageProcess::on_actionOpenFile_triggered()
{
}

void ImageProcess::updateMenus()
{
    // �˵���
    bool hasMdiChild = (activeSubMdiChild() != nullptr);
    ui->actionSave->setEnabled(hasMdiChild);
    ui->actionSave->setVisible(hasMdiChild);
    ui->actionClear->setEnabled(hasMdiChild);
    ui->actionClear->setVisible(hasMdiChild);

    ui->menuImageProcess->setEnabled(hasMdiChild);
    ui->menuImageEdit->setEnabled(hasMdiChild);
    ui->menuFourierTransform->setEnabled(hasMdiChild);
    ui->menuView->setEnabled(hasMdiChild);
}

subMdiQLabel* ImageProcess::createSubMdiChild()
{
    subMdiQLabel* child = new subMdiQLabel;
 //   pMdiArea->addSubWindow(child);
    return child;
}

void ImageProcess::systemTimeUpdate()
{
    QDateTime curTime = QDateTime::currentDateTime();
    QString strTime = curTime.toString(QString::fromLocal8Bit(" yyyy��MM��dd�� hh:mm:ss "));
    mTimerLable->setText(strTime);
   // this->statusBar()->addPermanentWidget(mTimerLable);
}

void ImageProcess::qimageInfo()
{
    auto curSub = activeSubMdiChild();
    if (curSub != nullptr) {
        QString hei = QString::number(curSub->currentImage().size().height());
        QString wid = QString::number(curSub->currentImage().size().width());
        mQImageInfo->setText(QString(" H: " + hei + ", W: " + wid + "  Type:" +
            curSub->currentFileSuffix(curSub->currentFile()).toUpper() + " "));    
    }
    else {
        mQImageInfo->setText(QString());
    }
}

void ImageProcess::on_actionClear_triggered()
{
    pMdiArea->closeAllSubWindows();
}

void ImageProcess::about()
{
    QMessageBox::about(this, tr("About This"),
        QString::fromLocal8Bit("���������--���Ѻ�\nд��2022��9�£���������Ȩ��\n���д������ϴ���\nhttps://github.com/Sam-Wong774\n��ӭһ�����ۣ�"));
}

void ImageProcess::closeEvent(QCloseEvent* event)
{
    pMdiArea->closeAllSubWindows();
    if (pMdiArea->currentSubWindow()) {
        event->ignore();
    }
    else {
        writeSettings();
        event->accept();
    }
}

void ImageProcess::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void ImageProcess::dropEvent(QDropEvent* event)
{
    QString fileName = event->mimeData()->urls().first().toLocalFile();
    openQImage(fileName);
}

int ImageProcess::initMainFrame()
{
    // ����
    this->setWindowTitle(QString::fromLocal8Bit("ͼ�������"));
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen || screen->size().isNull() || screen->size().isEmpty()) {
        return -1;
    }
    double initMinsizeRatio = 0.5;
    QSize screenSize(screen->size() * initMinsizeRatio);
    this->setMinimumSize(screenSize);
    // this->setWindowState(Qt::WindowState::WindowMaximized);
    this->setAcceptDrops(true);
    if (this->centralWidget()) {
        delete this->centralWidget();
    }
    setCentralWidget(pMdiArea);
    pMdiArea->setBackground(Qt::NoBrush);
    pMdiArea->setStyleSheet("QMdiArea{ "
   //" background-image: url();"
   //" background-repeat: no-repeat;"
   //" background-position: center;"
   " background-color: rgba(110, 173, 236, 255);"
   " }");

    // ״̬��
    // ���ý�������ɫ��(�ޱ߿���Բ����ʽ)
    mProgressBar->setStyleSheet("QProgressBar{border:1px;backgroud:white;} QProgressBar::chunk{border-radius:5px;bakcgroud:blue}");
    mProgressBar->hide();
    mStatusBarParaLayout->addStretch();
    // һ��Ҫ��˳�����������Ϣ���Ҳࣩ
    mStatusBarParaLayout->addWidget(mProgressBar);
    mStatusBarParaLayout->addWidget(mQImageInfo);   
    mStatusBarParaLayout->addWidget(mTimerLable);
    QWidget *statusWidget = new QWidget;
    //statusWidget->setWindowFlags(Qt::FramelessWindowHint);
    statusWidget->setLayout(mStatusBarParaLayout);
    statusWidget->setWindowFlags(Qt::FramelessWindowHint);
    this->statusBar()->addPermanentWidget(statusWidget);
    this->statusBar()->setSizeGripEnabled(true);

    // ������ͼ���С
    ui->mainToolBar->setIconSize(QSize(50,50));
    // �����ؼ�
    ui->actionOpen->setStatusTip(QString::fromLocal8Bit("�򿪵���ͼƬ"));
    ui->actionSave->setStatusTip(QString::fromLocal8Bit("���浥��ͼƬ"));
    ui->actionClear->setStatusTip(QString::fromLocal8Bit("�������ͼƬ����"));
    ui->actionAbout_This->setStatusTip(QString::fromLocal8Bit("���ڱ��������˵��"));
    ui->actionAbout_QT->setStatusTip(QString::fromLocal8Bit("����QT�汾˵��"));
    // �����ź����
    connect(mSystemTimer, SIGNAL(timeout()), this, SLOT(systemTimeUpdate()));
    connect(pMdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),  this, SLOT(qimageInfo()));
    connect(pMdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMenus()));
    connect(ui->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAbout_This, SIGNAL(triggered()), this, SLOT(about()));
    return 0;
}

void ImageProcess::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QGuiApplication::primaryScreen()->geometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
            (availableGeometry.height() - height()) / 2);
    }
    else {
        restoreGeometry(geometry);
    }
}

void ImageProcess::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool ImageProcess::openQImage(const QString& fileName)
{
    if (QMdiSubWindow* existing = findMdiChild(fileName)) {
        pMdiArea->setActiveSubWindow(existing);
        return true;
    }
    const bool succeeded = loadQImage(fileName);
    if (succeeded)
        statusBar()->showMessage(tr("QImage Loaded"), 2000);
    return succeeded;
}

bool ImageProcess::loadQImage(const QString& fileName)
{
    subMdiQLabel* child = createSubMdiChild();
    const bool succeeded = child->loadQImage(fileName);
    pMdiArea->addSubWindow(child);
    if (succeeded) {
        child->show();
    }   
    else {
        child->close();
    }     
    return succeeded;
}

bool ImageProcess::loadQImage(const QImage& showQImage)
{
    subMdiQLabel* child = createSubMdiChild();
    const bool succeeded = child->loadQImage(showQImage);
    pMdiArea->addSubWindow(child);
    if (succeeded) {
        child->show();
    }
    else {
        child->close();
    }
    return succeeded;
}

subMdiQLabel* ImageProcess::activeSubMdiChild() const
{
    if (QMdiSubWindow* activeSubWindow = pMdiArea->activeSubWindow()) {     
        return  qobject_cast<subMdiQLabel*>(activeSubWindow->widget());          
    }
    return nullptr;
}

QMdiSubWindow* ImageProcess::findMdiChild(const QString& fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach(QMdiSubWindow * window, pMdiArea->subWindowList()) {
        subMdiQLabel* mdiChild = qobject_cast<subMdiQLabel*>(window->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return window;
    }

    return nullptr;
}

//HWND ImageProcess::restrictMouseActivity()
//{ 
//    // ��������ڿͻ������������ʽ��ΪPointingHandCursor
//    setCursor(Qt::PointingHandCursor);
//    RECT rct;
//    rct.left = this->geometry().x() + pMdiArea->geometry().x();
//    rct.right = this->geometry().x() + pMdiArea->geometry().right();
//    rct.top = this->geometry().y() + pMdiArea->geometry().y();
//    rct.bottom = this->geometry().y() +  pMdiArea->geometry().bottom();
//    ClipCursor(&rct);
//    setCursor(Qt::CursorShape::PointingHandCursor);
//    return SetCapture(GetCapture());
//}

void ImageProcess::on_actionColor2Gray_triggered()
{
    if (!activeSubMdiChild()->currentImage().isNull()) {
        auto img = activeSubMdiChild()->currentImage();
        loadQImage(img.convertToFormat(QImage::Format::Format_Grayscale8));
    }
}

void ImageProcess::on_actionR_Channel_triggered()
{
    if (!activeSubMdiChild()->currentImage().isNull()) {
        auto img = activeSubMdiChild()->currentImage();
        if (img.format() == QImage::Format_Grayscale8) {
            return;
        }
        unsigned char* redData;                                 // �����ַ���ָ������blueData���ڴ洢��ɫ��������
        QPixmap pixmap(QPixmap::fromImage(img));                // ���ص�ǰͼ��,תΪPixmapͼ
        unsigned char* data = img.bits();                       // ָ��ǰͼ���һ������
        int width = img.width();                                // ͼ����
        int height = img.height();                              // ͼ��߶�
        int bytePerLine = img.bytesPerLine();                   // ͼ��ÿ���ֽ���
        redData = new unsigned char[(double)bytePerLine * (double)height];      // ���������С
        unsigned char red = 0;                                  // ��ɫ����
        for (int i = 0; i < height; i++)                        // ����ÿһ��
        {
            for (int j = 0; j < width; j++)                     // ����ÿһ��
            {
                red = *(data);                                  // ��ȡ��ǰ���ص���ɫ����
                redData[i * bytePerLine + j * 3] = red;         // ��ɫͨ����ԭֵ
                redData[i * bytePerLine + j * 3 + 1] = 0;
                redData[i * bytePerLine + j * 3 + 2] = 0;
                data += 4;                                      // ��������һ���ص� ƫ�������ر�ע��
            }
        }
        QImage redImage(redData, width, height, bytePerLine, QImage::Format_RGB888);
        //QPixmap pixmap2(QPixmap::fromImage(blueImage));       // ����ͼ�񴰿�
        loadQImage(redImage);
    }
}

void ImageProcess::on_actionG_Channel_triggered()
{
    if (!activeSubMdiChild()->currentImage().isNull()) {
        auto img = activeSubMdiChild()->currentImage();
        if (img.format() == QImage::Format_Grayscale8) {
            return;
        }
        unsigned char* greenData;                               // �����ַ���ָ������blueData���ڴ洢��ɫ��������
        QPixmap pixmap(QPixmap::fromImage(img));                // ���ص�ǰͼ��,תΪPixmapͼ
        unsigned char* data = img.bits();                       // ָ��ǰͼ���һ������
        int width = img.width();                                // ͼ����
        int height = img.height();                              // ͼ��߶�
        int bytePerLine = img.bytesPerLine();                   // ͼ��ÿ���ֽ���
        greenData = new unsigned char[(double)bytePerLine * (double)height];    // ���������С
        unsigned char green = 0;                                // ��ɫ����
        for (int i = 0; i < height; i++)                        // ����ÿһ��
        {
            for (int j = 0; j < width; j++)                     // ����ÿһ��
            {
                green = *(data);                                // ��ȡ��ǰ���ص���ɫ����
                greenData[i * bytePerLine + j * 3] = 0;         // ��ɫͨ����ԭֵ
                greenData[i * bytePerLine + j * 3 + 1] = green;
                greenData[i * bytePerLine + j * 3 + 2] = 0;
                data += 4;                                      // ��������һ���ص�
            }
        }
        QImage greenImage(greenData, width, height, bytePerLine, QImage::Format_RGB888);
        //QPixmap pixmap2(QPixmap::fromImage(blueImage));        // ����ͼ�񴰿�
        loadQImage(greenImage);
    }
}

void ImageProcess::on_actionB_Channel_triggered()
{
    if (!activeSubMdiChild()->currentImage().isNull()) {
        auto img = activeSubMdiChild()->currentImage();
        if (img.format() == QImage::Format_Grayscale8) {
            return;
        }
        unsigned char* blueData;                                // �����ַ���ָ������blueData���ڴ洢��ɫ��������
        QPixmap pixmap(QPixmap::fromImage(img));                // ���ص�ǰͼ��,תΪPixmapͼ
        unsigned char* data = img.bits();                       // ָ��ǰͼ���һ������
        int width = img.width();                                // ͼ����
        int height = img.height();                              // ͼ��߶�
        int bytePerLine = img.bytesPerLine();                   // ͼ��ÿ���ֽ���
        blueData = new unsigned char[(double)bytePerLine * (double)height];     // ���������С
        unsigned char blue = 0;                                 // ��ɫ����
        for (int i = 0; i < height; i++)                        // ����ÿһ��
        {
            for (int j = 0; j < width; j++)                     // ����ÿһ��
            {
                blue = *(data);                                 // ��ȡ��ǰ���ص���ɫ����
                blueData[i * bytePerLine + j * 3] = 0;          // ��ɫͨ����ԭֵ
                blueData[i * bytePerLine + j * 3 + 1] = 0;
                blueData[i * bytePerLine + j * 3 + 2] = blue;
                data += 4;                                      // ��������һ���ص�
            }
        }
        QImage blueImage(blueData, width, height, bytePerLine, QImage::Format_RGB888);
        //QPixmap pixmap2(QPixmap::fromImage(blueImage));       // ����ͼ�񴰿�
        loadQImage(blueImage);
    }
}

void ImageProcess::on_actionDFT_triggered()
{
    if (!activeSubMdiChild()->currentImage().isNull()) {
        auto img = activeSubMdiChild()->currentImage();
        myDFT dftImage(img);
        QTime time;
        time.start();
        dftImage.dftByOpenCV();
        if (!dftImage.getQImage().isNull()) {
            loadQImage(dftImage.getQImage());
            QMessageBox::information(NULL, tr("Time taked:"), tr("%1 ms").arg(time.elapsed()));
        }     
        //qDebug() << time.elapsed();
    }
    // ��ʱ����
    //#include <QDebug>  
    //#include <sys/time.h>  
    //double time_Start = (double)clock();
    //Function();
    //double time_End = (double)clock();
    //qDebug() << (time_End - time_Start) / 1000.0 << "s";
}

void ImageProcess::on_actionIDFT_triggered()
{
    if (!activeSubMdiChild()->currentImage().isNull()) {
        auto img = activeSubMdiChild()->currentImage();
        //ImageFourierTransfrom idftImage(img);
        QTime time;
        time.start();
        // if (idftImage.matIdftByOpencv()) {
        //    loadQImage(*idftImage.getQImage());
        // }
    }
}

void ImageProcess::on_actionFFT_triggered()
{
}

void ImageProcess::on_actionIFFT_triggered()
{
}

void ImageProcess::on_actionFFTW_triggered()
{
}

void ImageProcess::on_actionIFFTW_triggered()
{
}