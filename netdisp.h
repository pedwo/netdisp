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

#ifndef NETDISP_H
#define NETDISP_H

#include <qmainwindow.h>
#include <qboxlayout.h>
#include <qformlayout.h>
#include <qpushbutton.h>
#include <qdial.h>
#include <qicon.h>
#include <qlist.h>
#include <qsignalmapper.h>
#include <qtimer.h>

#define NR_PORTS		5

enum port_states {
	UNUSED = 0,		/* Disabled in HW due to pinmux */
	INACTIVE,		/* Link down */
	ACTIVE,			/* Link up */
	NR_PORT_STATES,
};

class NetDisp : public QMainWindow
{
	Q_OBJECT

public:
	NetDisp(QWidget *parent = 0);

public slots:
	void onPortPressed(int btn);
	void updatePortDisplay();
	void updatePortStatus();
	void updatePerfData();

private:
	QBoxLayout *createPerfDial(const QString & text);
	QBoxLayout *createRzn1PerfDials();
	QBoxLayout *createRzn1Port(int i);
	QBoxLayout *createRzn1Ports();
	void layoutWindow();

	QIcon *m_ethIcon[NR_PORT_STATES];
	int portStates[NR_PORTS];
	int newPortStates[NR_PORTS];
	QSignalMapper *m_signalMap;
	QList<QPushButton *> m_portBtns;
	QList<QDial *> m_cpuDials;
	QList<QDial *> m_portDials;
	QTimer *m_perfTimer;
	QTimer *m_portTimer;
};

#endif // NETDISP_H


