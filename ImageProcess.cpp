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
    // 菜单栏
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
    QString strTime = curTime.toString(QString::fromLocal8Bit(" yyyy年MM月dd日 hh:mm:ss "));
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
        QString::fromLocal8Bit("本软件作者--王佳豪\n写于2022年9月，保留著作权。\n所有代码已上传至\nhttps://github.com/Sam-Wong774\n欢迎一起讨论！"));
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
    // 窗体
    this->setWindowTitle(QString::fromLocal8Bit("图像处理软件"));
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

    // 状态栏
    // 设置进度条颜色与(无边框与圆角样式)
    mProgressBar->setStyleSheet("QProgressBar{border:1px;backgroud:white;} QProgressBar::chunk{border-radius:5px;bakcgroud:blue}");
    mProgressBar->hide();
    mStatusBarParaLayout->addStretch();
    // 一定要按顺序添加永久消息（右侧）
    mStatusBarParaLayout->addWidget(mProgressBar);
    mStatusBarParaLayout->addWidget(mQImageInfo);   
    mStatusBarParaLayout->addWidget(mTimerLable);
    QWidget *statusWidget = new QWidget;
    //statusWidget->setWindowFlags(Qt::FramelessWindowHint);
    statusWidget->setLayout(mStatusBarParaLayout);
    statusWidget->setWindowFlags(Qt::FramelessWindowHint);
    this->statusBar()->addPermanentWidget(statusWidget);
    this->statusBar()->setSizeGripEnabled(true);

    // 工具栏图标大小
    ui->mainToolBar->setIconSize(QSize(50,50));
    // 动作控件
    ui->actionOpen->setStatusTip(QString::fromLocal8Bit("打开单张图片"));
    ui->actionSave->setStatusTip(QString::fromLocal8Bit("保存单张图片"));
    ui->actionClear->setStatusTip(QString::fromLocal8Bit("清空所有图片窗口"));
    ui->actionAbout_This->setStatusTip(QString::fromLocal8Bit("关于本软件作者说明"));
    ui->actionAbout_QT->setStatusTip(QString::fromLocal8Bit("关于QT版本说明"));
    // 连接信号与槽
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
//    // 限制鼠标在客户区并将鼠标样式变为PointingHandCursor
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
        unsigned char* redData;                                 // 定义字符型指针数组blueData用于存储蓝色分量数据
        QPixmap pixmap(QPixmap::fromImage(img));                // 加载当前图像,转为Pixmap图
        unsigned char* data = img.bits();                       // 指向当前图像第一个像素
        int width = img.width();                                // 图像宽度
        int height = img.height();                              // 图像高度
        int bytePerLine = img.bytesPerLine();                   // 图像每行字节数
        redData = new unsigned char[(double)bytePerLine * (double)height];      // 设置数组大小
        unsigned char red = 0;                                  // 红色分量
        for (int i = 0; i < height; i++)                        // 遍历每一行
        {
            for (int j = 0; j < width; j++)                     // 遍历每一列
            {
                red = *(data);                                  // 获取当前像素点蓝色分量
                redData[i * bytePerLine + j * 3] = red;         // 红色通道赋原值
                redData[i * bytePerLine + j * 3 + 1] = 0;
                redData[i * bytePerLine + j * 3 + 2] = 0;
                data += 4;                                      // 更新至下一像素点 偏移量需特别注意
            }
        }
        QImage redImage(redData, width, height, bytePerLine, QImage::Format_RGB888);
        //QPixmap pixmap2(QPixmap::fromImage(blueImage));       // 更新图像窗口
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
        unsigned char* greenData;                               // 定义字符型指针数组blueData用于存储蓝色分量数据
        QPixmap pixmap(QPixmap::fromImage(img));                // 加载当前图像,转为Pixmap图
        unsigned char* data = img.bits();                       // 指向当前图像第一个像素
        int width = img.width();                                // 图像宽度
        int height = img.height();                              // 图像高度
        int bytePerLine = img.bytesPerLine();                   // 图像每行字节数
        greenData = new unsigned char[(double)bytePerLine * (double)height];    // 设置数组大小
        unsigned char green = 0;                                // 绿色分量
        for (int i = 0; i < height; i++)                        // 遍历每一行
        {
            for (int j = 0; j < width; j++)                     // 遍历每一列
            {
                green = *(data);                                // 获取当前像素点蓝色分量
                greenData[i * bytePerLine + j * 3] = 0;         // 绿色通道赋原值
                greenData[i * bytePerLine + j * 3 + 1] = green;
                greenData[i * bytePerLine + j * 3 + 2] = 0;
                data += 4;                                      // 更新至下一像素点
            }
        }
        QImage greenImage(greenData, width, height, bytePerLine, QImage::Format_RGB888);
        //QPixmap pixmap2(QPixmap::fromImage(blueImage));        // 更新图像窗口
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
        unsigned char* blueData;                                // 定义字符型指针数组blueData用于存储蓝色分量数据
        QPixmap pixmap(QPixmap::fromImage(img));                // 加载当前图像,转为Pixmap图
        unsigned char* data = img.bits();                       // 指向当前图像第一个像素
        int width = img.width();                                // 图像宽度
        int height = img.height();                              // 图像高度
        int bytePerLine = img.bytesPerLine();                   // 图像每行字节数
        blueData = new unsigned char[(double)bytePerLine * (double)height];     // 设置数组大小
        unsigned char blue = 0;                                 // 绿色分量
        for (int i = 0; i < height; i++)                        // 遍历每一行
        {
            for (int j = 0; j < width; j++)                     // 遍历每一列
            {
                blue = *(data);                                 // 获取当前像素点蓝色分量
                blueData[i * bytePerLine + j * 3] = 0;          // 绿色通道赋原值
                blueData[i * bytePerLine + j * 3 + 1] = 0;
                blueData[i * bytePerLine + j * 3 + 2] = blue;
                data += 4;                                      // 更新至下一像素点
            }
        }
        QImage blueImage(blueData, width, height, bytePerLine, QImage::Format_RGB888);
        //QPixmap pixmap2(QPixmap::fromImage(blueImage));       // 更新图像窗口
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
    // 计时方法
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