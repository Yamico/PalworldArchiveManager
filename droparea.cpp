// droparea.cpp
#include "droparea.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMimeData>

DropArea::DropArea(QWidget *parent) : QWidget(parent) {

    setAcceptDrops(true);
    auto layout = new QVBoxLayout(this);
    auto label = new QLabel(tr("可拖拽进此处"));
    label->setAlignment(Qt::AlignCenter);
    // 设置 QLabel 的样式
    label->setStyleSheet(
        "QLabel {"
        "  font-size: 16pt;" // 设置字体大小
        "  color: gray;"    // 设置字体颜色
        "  border: 2px dashed gray;"  // 设置边框样式
        "  border-radius: 10px;"      // 设置边框圆角
        "}"
        );

    layout->addWidget(label);
}

void DropArea::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        // 更改提示信息
        static_cast<QLabel *>(layout()->itemAt(0)->widget())->setText(tr("放进此处"));
    }
}

void DropArea::dragLeaveEvent(QDragLeaveEvent *event) {
    Q_UNUSED(event);
    // 恢复原始提示信息
    static_cast<QLabel *>(layout()->itemAt(0)->widget())->setText(tr("可拖拽进此处"));
}

void DropArea::dropEvent(QDropEvent *event) {
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty() && urls.first().isLocalFile()) {
        QString folderPath = urls.first().toLocalFile();
        emit folderDropped(folderPath); // 发送信号
    }
}

void DropArea::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit clicked();  // 当鼠标点击时发出信号
}


