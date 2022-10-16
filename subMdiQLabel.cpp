#include "stdafx.h"
#include "subMdiQLabel.h"
#include <qimagereader.h>

subMdiQLabel::subMdiQLabel()
{
	setAttribute(Qt::WA_DeleteOnClose);
}

void subMdiQLabel::newFile()
{
	static int sequenceNumber = 1;
	// isUntitled = TRUE;
	curQImage = QImage(QSize(256, 256), QImage::Format_RGB888);
	curQImage.fill(QColor(Qt::GlobalColor::white));
	setWindowTitle(tr("new%1").arg(sequenceNumber++));

}

bool subMdiQLabel::loadQImage(const QString& filePath)
{
	if (filePath.isNull()) {
		return false;
	}
	curFile = filePath;
	QImageReader reader(filePath);
	reader.setDecideFormatFromContent(true);
	if (!reader.canRead() || !reader.read(&curQImage)) {
		return false;
	}
	this->setPixmap(QPixmap::fromImage(curQImage));
	this->setWindowTitle(currentFileName(filePath));
	return true;
}

bool subMdiQLabel::loadQImage(const QImage& showImage)
{
	if (showImage.isNull()) {
		return false;
	}
	curQImage = showImage.copy();		//	¸±±¾
	this->setPixmap(QPixmap::fromImage(curQImage));
	static int sequenceNumber = 1;
	setWindowTitle(tr("Image%1").arg(sequenceNumber++));
	return true;
}

bool subMdiQLabel::save()
{
	if (currentImage().isNull()) {
		return false;
	}
	static int cnt = 1;
	QString fileName = QString("../%1-%2").arg(currentFile()).arg(cnt++);
	static const QString strSavelist("Image(*.bmp);;Image(*.png);;Image(*.jpg)");
	QString fileSavePath = QFileDialog::getSaveFileName(this, tr("Save Image"), fileName, strSavelist);
	if (!currentImage().save(fileSavePath)) {
		qDebug() << "Save Error";
		return false;
	}
	return true;
}

void subMdiQLabel::resizeEvent(QResizeEvent* ev)
{
	this->setScaledContents(true);
	this->resize(this->size());
}










