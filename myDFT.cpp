#include "stdafx.h"
#include "myDFT.h"

myDFT& myDFT::operator=(const myDFT& input)
{
	// TODO: �ڴ˴����� return ���
	// ���
	if (image) delete image;
	image = new QImage(input.getQImage().copy());
	return *this;
}

myDFT& myDFT::operator=(myDFT&& input) noexcept
{
	// TODO: �ڴ˴����� return ���
	if (this != &input) {
		if (image) delete image;
		image = input.image;
		// ǳ����
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
		cv::cvtColor(img_mat, img_mat, cv::COLOR_RGB2GRAY);	// �ҶȻ�
	}

	//��1��������ͼ����������ѵĳߴ磬�߽���0����
	//Ŀ����Ϊ��ͼ����2,3,5�������ӿ�����ٶ�
	int m = cv::getOptimalDFTSize(img_mat.rows);
	int n = cv::getOptimalDFTSize(img_mat.cols);
	//����ӵ����س�ʼ��Ϊ0.
	Mat padded;
	cv::copyMakeBorder(img_mat, padded, 0, m - img_mat.rows, 0, n - img_mat.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	//��2��Ϊ����Ҷ�任�Ľ��(ʵ�����鲿)����洢�ռ䡣
	//����Ҷ����Ǹ���������ÿ��ԭͼ��ֵ���������2��ͼ��ֵ
	//���⣬Ƶֵ��ΧԶ���ڿռ�ֵ��Χ�����Դ���float��ʽ��������һ��ͨ���洢��������
	//��planes������Ϻϲ���һ����ͨ��������complexI
	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexI;
	cv::merge(planes, 2, complexI);	// �ϲ���һ��mat
	cv::dft(complexI, complexI);	// dft��Ҫһ��2ͨ����mat

	//��3��������ת��Ϊ��ֵ����=> log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	cv::split(complexI, planes);// ����ͨ������complexI����ɼ�����ͨ�����飬planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	cv::magnitude(planes[0], planes[1], planes[0]);//   planes[0] = magnitude 
	Mat magnitudeImage = planes[0];// ��ģ

	//��4�����ж����߶�(logarithmic scale)����
	//����Ҷ�任����ֵ��Χ�󵽲��ʺ�����Ļ��ʾ����ֵΪ�׵㣬��ֵΪ�ڵ㣬���Զ����任�߶�
	magnitudeImage += Scalar::all(1);
	log(magnitudeImage, magnitudeImage);//����Ȼ����

	//��5�����к��طֲ�����ͼ����
	//���ڶ�������չ��ͼ������ӵ������޳�
	//���������л������У�����Ƶ�ײü�   
	magnitudeImage = magnitudeImage(Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));
	//�������и���Ҷͼ���е����ޣ�ʹ��ԭ��λ��ͼ������ 
	int cx = magnitudeImage.cols / 2;
	int cy = magnitudeImage.rows / 2;
	Mat q0(magnitudeImage, Rect(0, 0, cx, cy));   // ROI���������
	Mat q1(magnitudeImage, Rect(cx, 0, cx, cy));  // ROI���������
	Mat q2(magnitudeImage, Rect(0, cy, cx, cy));  // ROI���������
	Mat q3(magnitudeImage, Rect(cx, cy, cx, cy)); // ROI���������
	//�������ޣ����������½��н�����
	Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	//�������ޣ����������½��н�����
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	//��6����һ������0��1֮��ĸ���ֵ������任Ϊ���ӵ�ͼ���ʽ
	//�˾�����OpenCV2��Ϊ��
	//normalize(magnitudeImage, magnitudeImage, 0, 1, CV_MINMAX); 
	//�˾�����OpenCV3��Ϊ:
	normalize(magnitudeImage, magnitudeImage, 0, 1, NORM_MINMAX);

	//��7����ʾЧ��ͼ
	//imshow("Ƶ�׷�ֵ", magnitudeImage);
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
		cv::cvtColor(img_mat, img_mat, cv::COLOR_RGB2GRAY);	// �ҶȻ�
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

