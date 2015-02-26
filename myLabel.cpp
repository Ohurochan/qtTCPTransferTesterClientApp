#include "myLabel.h"

#include <QWidget>

#ifdef _DEBUG
#include <QDebug>
#endif

myLabel::myLabel(QWidget *parent) :
    QLabel(parent)
{
}

void myLabel::fixFontSize()
{
     QFont font = this->font();
     QRect cRect = this->contentsRect();

    if( this->text().isEmpty() )
            return;

     int fontSize = 1;

      while( true )
      {
                  QFont f(font);
                       f.setPixelSize( fontSize );
                  QRect r = QFontMetrics(f).boundingRect( this->text() );
#ifdef _DEBUG
                  qDebug() << r.height() << cRect.height() << r.width() << cRect.width();
#endif
                  if (r.height() < cRect.height() && r.width() < cRect.width() )
                        fontSize++;
                  else
                        break;
      }

     font.setPixelSize(fontSize);
     this->setFont(font);
}
