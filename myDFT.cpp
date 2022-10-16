#include "stdafx.h"
#include "myDFT.h"

myDFT& myDFT::operator=(const myDFT& input)
{
	// TODO: 在此处插入 return 语句
	// 深拷贝
	if (image) delete image;
	image = new QImage(input.getQImage().copy());
	return *this;
}

myDFT& myDFT::operator=(myDFT&& input) noexcept
{
	// TODO: 在此处插入 return 语句
	if (this != &input) {
		if (image) delete image;
		image = input.image;
		// 浅拷贝
		// image = new QImage(input.getQImage().bits(), input.getQImage().width(), input.getQImage().height(), input.getQImage().format());
		input.image = nullptr;
	}
	return *this;
}

QImage myDFT::cvMat2QImage(const Mat& mat, bool clone, bool rb_swap)
{
    const uchar* pSrc = (const uchar*)mat.data;
    if (mat.type() == CV_8UC1) {
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        if (clone) return image.copy();
        return image;
    }
    else if (mat.type() == CV_8UC3) {
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        if (clone) {
            if (rb_swap) return image.rgbSwapped();
            return image.copy();
        }
        else {
            if (rb_swap) {
                cvtColor(mat, mat, COLOR_BGR2RGB);
            }
            return  image;
        }
    }
    else if (mat.type() == CV_8UC4) {
        qDebug() << "CV_8UC4";
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        if (clone) return image.copy();
        return image;
    }
    else {
        qDebug() << "Error: Mat could NOT be converted to QImage";
        return QImage();
    }
}

Mat myDFT::QImage2cvMat(QImage& image, bool clone, bool rb_swap)
{
    Mat mat;
    switch (image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        if (clone) mat = mat.clone();
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        if (clone) mat = mat.clone();
        if (rb_swap) cv::cvtColor(mat, mat, COLOR_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
    case QImage::Format_Grayscale8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        if (clone) mat = mat.clone();
        break;
    default:
        break;
    }
    return mat;
}

void myDFT::dftByOpenCV()
{
	if (image->isNull()) return;
	Mat img_mat;
	if (image->format() == QImage::Format_RGB888)
	{
		img_mat = QImage2cvMat(*image);
	}
	else
	{
		img_mat = QImage2cvMat(*image, true, false);
	}
	//*img_mat = Mat2QImage::QImage2cvMat(*img_qimage, true, false);
	if (img_mat.channels() != 1) {
		cv::cvtColor(img_mat, img_mat, cv::COLOR_RGB2GRAY);	// 灰度化
	}

	//【1】将输入图像延扩到最佳的尺寸，边界用0补充
	//目的是为了图像变成2,3,5倍数，加快计算速度
	int m = cv::getOptimalDFTSize(img_mat.rows);
	int n = cv::getOptimalDFTSize(img_mat.cols);
	//将添加的像素初始化为0.
	Mat padded;
	cv::copyMakeBorder(img_mat, padded, 0, m - img_mat.rows, 0, n - img_mat.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	//【2】为傅立叶变换的结果(实部和虚部)分配存储空间。
	//傅里叶结果是复数，对于每个原图像值，结果会有2个图像值
	//此外，频值范围远大于空间值范围，所以存在float格式，并增加一个通道存储复数部分
	//将planes数组组合合并成一个多通道的数组complexI
	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexI;
	cv::merge(planes, 2, complexI);	// 合并成一个mat
	cv::dft(complexI, complexI);	// dft需要一个2通道的mat

	//【3】将复数转换为幅值，即=> log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	cv::split(complexI, planes);// 将多通道数组complexI分离成几个单通道数组，planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	cv::magnitude(planes[0], planes[1], planes[0]);//   planes[0] = magnitude 
	Mat magnitudeImage = planes[0];// 求模

	//【4】进行对数尺度(logarithmic scale)缩放
	//傅里叶变换幅度值范围大到不适合在屏幕显示，高值为白点，低值为黑点，所以对数变换尺度
	magnitudeImage += Scalar::all(1);
	log(magnitudeImage, magnitudeImage);//求自然对数

	//【5】剪切和重分布幅度图象限
	//将第二步中延展的图像，新添加的像素剔除
	//若有奇数行或奇数列，进行频谱裁剪   
	magnitudeImage = magnitudeImage(Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));
	//重新排列傅立叶图像中的象限，使得原点位于图像中心 
	int cx = magnitudeImage.cols / 2;
	int cy = magnitudeImage.rows / 2;
	Mat q0(magnitudeImage, Rect(0, 0, cx, cy));   // ROI区域的左上
	Mat q1(magnitudeImage, Rect(cx, 0, cx, cy));  // ROI区域的右上
	Mat q2(magnitudeImage, Rect(0, cy, cx, cy));  // ROI区域的左下
	Mat q3(magnitudeImage, Rect(cx, cy, cx, cy)); // ROI区域的右下
	//交换象限（左上与右下进行交换）
	Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	//交换象限（右上与左下进行交换）
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	//【6】归一化，用0到1之间的浮点值将矩阵变换为可视的图像格式
	//此句代码的OpenCV2版为：
	//normalize(magnitudeImage, magnitudeImage, 0, 1, CV_MINMAX); 
	//此句代码的OpenCV3版为:
	normalize(magnitudeImage, magnitudeImage, 0, 1, NORM_MINMAX);

	//【7】显示效果图
	//imshow("频谱幅值", magnitudeImage);
	//qDebug() << magnitudeImage.type() << endl;
	// magnitudeImage.copyTo(img_mat);
	magnitudeImage.convertTo(img_mat, CV_8UC1, 255.0);
	*image = cvMat2QImage(img_mat);

}

void myDFT::idftByOpenCV()
{
	if (image->isNull()) return;
	Mat img_mat;
	if (image->format() == QImage::Format_RGB888)
	{
		img_mat = QImage2cvMat(*image);
	}
	else
	{
		img_mat = QImage2cvMat(*image, true, false);
	}
	//*img_mat = Mat2QImage::QImage2cvMat(*img_qimage, true, false);
	if (img_mat.channels() != 1) {
		cv::cvtColor(img_mat, img_mat, cv::COLOR_RGB2GRAY);	// 灰度化
	}

}

void myDFT::fftByOpenCV()
{
}

void myDFT::ifftByOpenCV()
{
}

void myDFT::fftwByFFTW()
{
}

