/*
 * Copyright (c) 2017 Renesas Electronics Europe Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "netdisp.h"
#include <QtWidgets/QApplication>

#define SHOW_FULLSCREEN

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	NetDisp w;

	QString style =
		"QPushButton {"
			"border-width: 0px;"
			"border-radius: 0px;"
		"}"
		"QPushButton:pressed {"
			"border-width: 0px;"
			"border-radius: 0px;"
		"}"
		"QLabel {"
			"color: black;"
			"font-size: 42pt;"
		"}"
		"QMainWindow {"
			"background-color: lightGray;"
		"}";

	a.setStyleSheet(style);

#ifdef SHOW_FULLSCREEN 
	w.showFullScreen();
#else
	w.show();
#endif

	return a.exec();
}

