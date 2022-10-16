#pragma once

#include <qlabel.h>
#include <qimage.h>
#include <qevent.h>


class subMdiQLabel :
    public QLabel
{
    Q_OBJECT
public:
    subMdiQLabel();
    void newFile();
    bool loadQImage(const QString& fileName);
    bool loadQImage(const QImage& showImage);
    bool save();
    QImage currentImage() const { return curQImage; }
    QString currentFile() const { return curFile; }
    inline QString currentFileName(const QString& filePath) const {
        return QFileInfo(filePath).fileName().remove(currentFileSuffix(filePath)).remove(".");
    }
    inline QString currentFileSuffix(const QString& filePath) const {
        return QFileInfo(filePath).suffix();
    };

protected:

    void resizeEvent(QResizeEvent* ev) override;
private slots:

private:
    QString curFile;
    QImage curQImage;
};

