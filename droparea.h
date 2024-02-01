// droparea.h
#ifndef DROPAREA_H
#define DROPAREA_H

#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>

class DropArea : public QWidget {
    Q_OBJECT

public:
    explicit DropArea(QWidget *parent = nullptr);

signals:
    void folderDropped(const QString &folderPath);
    void clicked();  // 添加一个新的信号

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;  // 重写 mousePressEvent
};

#endif // DROPAREA_H
