#pragma once
#ifndef __ImageDFT
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <fftw3.h>
#include <qimage.h>
#include <qalgorithms.h>
using namespace cv;
class myDFT
{
public:
	myDFT() : image(new QImage) {};
	virtual ~myDFT() {
		if (image) delete image;
	};
	myDFT(const QImage& qimage) : image(new QImage(qimage.copy())) {};
	myDFT(const myDFT& input) : image(new QImage(input.getQImage().copy())) {};
	myDFT(myDFT&& input) noexcept { *this = std::move(input); }
	myDFT& operator=(const myDFT& input);
	myDFT& operator=(myDFT&& input) noexcept;
	virtual QImage getQImage() const { return *image; }
private:
	/*
	* @brief �� OpenCV��cv::Mat ����ͼ��ת��ΪQImage����
	* @param mat ��ת����ͼ��֧��CV_8UC1��CV_8UC3��CV_8UC4��������
	* @param clone true ��ʾ��Mat�������ڴ棬�������ɵ�mat����Ӱ��ԭʼͼ��false�����mat�����ڴ�
	* @param rb_swap    ֻ��� CV_8UC3 ��ʽ��true�����R��B ��RGB -> BGR����������ڴ�Ļ�ԭʼͼ��Ҳ�ᷢ���仯
	* @return ת�����QImageͼ��
	*/
	static QImage cvMat2QImage(const Mat& mat, bool clone = true, bool rb_swap = true);
	/*
	* @brief �� QImage ����ͼ��ת��ΪOpenCV�� cv::Mat ����
	* @param QImage ��ת����ͼ��֧��Format_Indexed8��Format_Grayscale��24λ��ɫ��32λ��ɫ
	* @param clone true ��ʾ��QImage�������ڴ棬�������ɵ�mat����Ӱ��ԭʼͼ��false�����QImage�����ڴ�
	* @param rb_swap ֻ��� RGB888 ��ʽ��true�����R��B ��RGB -> BGR����������ڴ�Ļ�ԭʼͼ��Ҳ�ᷢ���仯
	* @return ת�����cv::Matͼ��
	*/
	static Mat QImage2cvMat(QImage& image, bool clone = true, bool rb_swap = true);
public:
	void dftByOpenCV();
	void idftByOpenCV();
	void fftByOpenCV();
	void ifftByOpenCV();
	void fftwByFFTW();


private:
	QImage *image;
};
#endif // !__ImageDFT