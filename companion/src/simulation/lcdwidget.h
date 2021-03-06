/*
 * Author - Bertrand Songis <bsongis@gmail.com>
 * 
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef lcd_widget_h
#define lcd_widget_h

#include <QWidget>
#include "appdata.h"

class lcdWidget : public QWidget {
  public:
    lcdWidget(QWidget * parent = 0):
      QWidget(parent),
      lcdBuf(NULL),
      previousBuf(NULL),
      lightEnable(false)
    {
    }

    ~lcdWidget()
    {
      if (previousBuf)
        free(previousBuf);
    }

    void setData(unsigned char *buf, int width, int height, int depth=1)
    {
      lcdBuf = buf;
      lcdWidth = width;
      lcdHeight = height;
      lcdDepth = depth;
      lcdSize = (width * ((height+7)/8)) * depth;
      previousBuf = (unsigned char *)malloc(lcdSize);
      memset(previousBuf, 0, lcdSize);
    }

    void setBackgroundColor(int red, int green, int blue)
    {
      _r = red;
      _g = green;
      _b = blue;
    }

    void makeScreenshot(const QString & fileName)
    {
      QPixmap buffer(2*lcdWidth, 2*lcdHeight);
      QPainter p(&buffer);
      doPaint(p);
      bool toclipboard = g.snapToClpbrd();
      if (toclipboard) {
        QApplication::clipboard()->setPixmap( buffer );
      }
      else {
        QString path = g.snapshotDir();
        if (path.isEmpty() || !QDir(path).exists()) {
          path = ".";
        }
        path.append(QDir::separator() + fileName);
        qDebug() << "Screenshot" << path;
        buffer.toImage().save(path);
      }
    }

    void onLcdChanged(bool light)
    {
      if (light != lightEnable || memcmp(previousBuf, lcdBuf, lcdSize)) {
        lightEnable = light;
        memcpy(previousBuf, lcdBuf, lcdSize);
        update();
      }
    }

    virtual void mousePressEvent(QMouseEvent * event)
    {
      setFocus();
      QWidget::mousePressEvent(event);
    }

  protected:

    int lcdWidth;
    int lcdHeight;
    int lcdDepth;
    int lcdSize;

    unsigned char *lcdBuf;
    unsigned char *previousBuf;

    bool lightEnable;
    int _r, _g, _b;

    inline void doPaint(QPainter & p)
    {
      QRgb rgb;

      if (lightEnable)
        rgb = qRgb(_r, _g, _b);
      else
        rgb = qRgb(161, 161, 161);

      p.setBackground(QBrush(rgb));
      p.eraseRect(0, 0, 2*lcdWidth, 2*lcdHeight);

      if (lcdBuf) {
        if (lcdDepth == 1) {
          rgb = qRgb(0, 0, 0);
          p.setPen(rgb);
          p.setBrush(QBrush(rgb));
        }

        unsigned int previousDepth = 0xFF;

        for (int y=0; y<lcdHeight; y++) {
          unsigned int idx = (y*lcdDepth/8)*lcdWidth;
          unsigned int mask = (1 << (y%8));
          for (int x=0; x<lcdWidth; x++, idx++) {
            if (lcdDepth == 1) {
              if (lcdBuf[idx] & mask) {
                p.drawRect(2*x, 2*y, 1, 1);
              }
            }
            else {
              unsigned int z = (y & 1) ? (lcdBuf[idx] >> 4) : (lcdBuf[idx] & 0x0F);
              if (z) {
                if (z != previousDepth) {
                  previousDepth = z;
                  if (lightEnable)
                    rgb = qRgb(_r-(z*_r)/15, _g-(z*_g)/15, _b-(z*_b)/15);
                  else
                    rgb = qRgb(161-(z*161)/15, 161-(z*161)/15, 161-(z*161)/15);
                  p.setPen(rgb);
                  p.setBrush(QBrush(rgb));
                }
                p.drawRect(2*x, 2*y, 1, 1);
              }
            }
          }
        }
      }
    }

    void paintEvent(QPaintEvent*)
    {
      QPainter p(this);
      doPaint(p);
    }

};

#endif
