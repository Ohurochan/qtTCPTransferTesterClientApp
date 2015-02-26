#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>

class myLabel : public QLabel
{
    Q_OBJECT
public:
    explicit myLabel(QWidget *parent = 0);

signals:

public slots:
    void fixFontSize();
};

#endif // MYLABEL_H
