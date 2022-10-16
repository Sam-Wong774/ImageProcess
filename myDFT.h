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
	* @brief 将 OpenCV的cv::Mat 类型图像转换为QImage类型
	* @param mat 待转换的图像，支持CV_8UC1、CV_8UC3、CV_8UC4三种类型
	* @param clone true 表示与Mat不共享内存，更改生成的mat不会影响原始图像，false则会与mat共享内存
	* @param rb_swap    只针对 CV_8UC3 格式，true会调换R与B ：RGB -> BGR，如果共享内存的话原始图像也会发生变化
	* @return 转换后的QImage图像
	*/
	static QImage cvMat2QImage(const Mat& mat, bool clone = true, bool rb_swap = true);
	/*
	* @brief 将 QImage 类型图像转换为OpenCV的 cv::Mat 类型
	* @param QImage 待转换的图像，支持Format_Indexed8、Format_Grayscale、24位彩色、32位彩色
	* @param clone true 表示与QImage不共享内存，更改生成的mat不会影响原始图像，false则会与QImage共享内存
	* @param rb_swap 只针对 RGB888 格式，true会调换R与B ：RGB -> BGR，如果共享内存的话原始图像也会发生变化
	* @return 转换后的cv::Mat图像
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