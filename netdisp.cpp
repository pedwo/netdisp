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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "netdisp.h"
#include <qboxlayout.h>
#include <qformlayout.h>
#include <qpalette.h>
#include <qlabel.h>
#include <qevent.h>
#include <qdebug.h>
#include <cpu_idle.h>

static int fifofd;
static char *fifofile = "/tmp/switchport-status";
static char fifobuf[1024];

/* independent timers to update the performance dials and port status */
#define PERF_UPDATE_MS	100
#define PORT_UPDATE_MS	100

NetDisp::NetDisp(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);

	layoutWindow();
}

void NetDisp::updatePortDisplay()
{
	int refresh = 0;

	/* Detect changed port status and handle it */
	for (int i = 0; i < NR_PORTS; i++) {
		if (portStates[i] == newPortStates[i])
			continue;
		portStates[i] = newPortStates[i];
		QPushButton *button = m_portBtns.at(i);
		button->setIcon(*m_ethIcon[portStates[i]]);

		/* Update the port speed label */
		QLabel *label = m_portLabels.at(i);
		if (portStates[i] == ACTIVE) {
			QString s = QString::number(portSpeed[i]);
			s.append(" Mbps");
			label->setText(s);
		} else {
			QString s = QString("No link");
			label->setText(s);
		}

		refresh = 1;
	}

	if (refresh)
		update();
}

void NetDisp::onPortPressed(int port)
{
	if (port == 0 || port >= NR_PORTS)
		return;

	/* TODO pop up a box with further info when port is pressed? */

	/* For now, enable/disable the port */
	if (newPortStates[port] == UNUSED)
		newPortStates[port] = INACTIVE;
	else
		newPortStates[port] = UNUSED;

	updatePortDisplay();
}

/* Rather slow way to read until newline... */
static int readline(int fd, char *buffer, size_t max_len)
{
	char c;
	int nr = 0;

	while (read(fd, &c, 1) > 0) {
		if (c == '\n')
			break;
		buffer[nr++] = c;
		if (nr == max_len)
			break;
	}
	return nr;
}

static unsigned char count;
void NetDisp::updatePortStatus()
{
	for (;;) {
		int len;
		unsigned int port_nr, link_up, speed, rx_load, tx_load;

		/* Get port data from named pipe */
		len = readline(fifofd, fifobuf, sizeof(fifobuf));
		if (len <= 0)
			break;

		sscanf(fifobuf, "port %u, link %u, speed %u, tx %u, rx %u\n", &port_nr, &link_up, &speed, &tx_load, &rx_load);

		/* Switch port 4 is actually the management port... */
		port_nr = (port_nr + 1) % NR_PORTS;
		if (link_up) {
			printf("netdisp port %u, link %u, speed %u, tx %u, rx %u\n",
					port_nr, link_up, speed, tx_load, rx_load);
		}

		if (link_up) {
			unsigned int max_bytes_per_sec = speed * 1000 * 1000 / 8;
			unsigned int percent_load = (100 * (tx_load + rx_load)) / max_bytes_per_sec;
			newPortStates[port_nr] = ACTIVE;
			portSpeed[port_nr] = speed;
			m_portDials.at(port_nr)->setValue(percent_load);
			printf("netdisp port %u, load %u\n", port_nr, percent_load);
		} else {
			newPortStates[port_nr] = INACTIVE;
			m_portDials.at(port_nr)->setValue(0);
		}
	}

	updatePortDisplay();
}

void NetDisp::updatePerfData()
{
	/* Get Cortex A7 loads */
	for (int i = 0; i < 2; i++) {
		/* Calc percent cpu load (3/4 decay on last value) */
		int busy = PERF_UPDATE_MS - get_cpu_idle_ms(i, 3, 4);
		int load = (busy * 100) / PERF_UPDATE_MS;
		/* QDial doesn't like the value set to 100 */
		if (load > 95)
			load = 95;
		m_cpuDials.at(i)->setValue(load);
	}

	/* TODO get Cortex M3 load somehow (dummy for now) */
	QDial *dial = m_cpuDials.at(2);
	dial->setValue(3 + rand() % 4);
}

/* Helper funcs */
QBoxLayout *NetDisp::createPerfDial(const QString & text)
{
	QBoxLayout *layout = new QVBoxLayout;
	QDial *dial = new QDial();

	dial->setFixedSize(140, 140);
	dial->setNotchesVisible(true);
	QPalette palette;
	palette.setColor(QPalette::Button, Qt::darkRed);
	dial->setPalette(palette);
	m_cpuDials.append(dial);
	layout->addWidget(dial);

	QLabel *label = new QLabel(text);
	label->setFixedSize(140, 40);
	label->setAlignment(Qt::AlignCenter);
	layout->addWidget(label);

	return layout;
}

QBoxLayout *NetDisp::createRzn1PerfDials()
{
	QBoxLayout *layout = new QHBoxLayout;

	layout->addStretch();
	layout->addLayout(createPerfDial("Cortex A7-1"));
	layout->addSpacing(20);
	layout->addLayout(createPerfDial("Cortex A7-2"));
	layout->addSpacing(20);
	layout->addLayout(createPerfDial("R-IN Engine (CM3)"));
	layout->addStretch();

	return layout;
}

QBoxLayout *NetDisp::createRzn1Port(int i, const QString & text)
{
	QBoxLayout *layout = new QVBoxLayout;

	QDial *dial = new QDial();
	dial->setFixedSize(110, 110);
	dial->setNotchesVisible(true);
	QPalette palette;
	palette.setColor(QPalette::Button, Qt::darkRed);
	dial->setPalette(palette);
	m_portDials.append(dial);
	layout->addWidget(dial);
	layout->setAlignment(dial, Qt::AlignCenter);
	layout->addSpacing(10);

	QLabel *label = new QLabel(text);
	label->setAlignment(Qt::AlignCenter);
	layout->addWidget(label);
	m_portLabels.append(label);

	QPushButton *btn = new QPushButton();
	btn->setFixedSize(120, 120);
	m_signalMap->setMapping(btn, i);
	connect(btn, SIGNAL(pressed()), m_signalMap, SLOT(map()));
	m_portBtns.append(btn);
	btn->setIcon(*m_ethIcon[portStates[i]]);
	btn->setIconSize(QSize(btn->width(), btn->height()));
	layout->addWidget(btn);

	return layout;
}

QBoxLayout *NetDisp::createRzn1Ports()
{
	QBoxLayout *layout = new QHBoxLayout;

	/* Load the images we use for the ethernet port icons */
	m_ethIcon[UNUSED]   = new QIcon("port-unused.png");
	m_ethIcon[INACTIVE] = new QIcon("port-inactive.png");
	m_ethIcon[ACTIVE]   = new QIcon("port-active.png");

	layout->addStretch();
	layout->addLayout(createRzn1Port(0, "Mgmt 1Gpbs"));
	/* Add a spacer after Port 1 as this is the Management Port */
	layout->addSpacing(40);

	for (int i = 1; i < NR_PORTS; i++) {
		layout->addSpacing(20);
		layout->addLayout(createRzn1Port(i, "No link"));
	}
	layout->addStretch();

	return layout;
}

void NetDisp::layoutWindow()
{
	QBoxLayout *mainLayout;
	m_signalMap = new QSignalMapper(this);
	connect(m_signalMap, SIGNAL(mapped(int)), SLOT(onPortPressed(int)));

	/* Open named pipe to get the port data */
	fifofd = open(fifofile, O_RDONLY | O_NONBLOCK);

	/* Set up port status */
	/* TODO We use hardcoded unused ports due to pinmux */
	portStates[0] = ACTIVE;		/* Management port */
	portStates[1] = INACTIVE;
	portStates[2] = INACTIVE;
	portStates[3] = INACTIVE;
	portStates[4] = INACTIVE;
	for (int i = 0; i < NR_PORTS; i++)
		newPortStates[i] = portStates[i];

	portSpeed[0] = 1000;		/* Management port */
	/* TODO dummy data for port speed */
	portSpeed[1] = 100;
	portSpeed[2] = 100;
	portSpeed[3] = 1000;
	portSpeed[4] = 10;

	/* RZ logo */
	QPushButton *rzn1Logo = new QPushButton();
	rzn1Logo->setFixedSize(216, 77);
	rzn1Logo->setIcon(QIcon("Renesas-RZ.png"));
	rzn1Logo->setIconSize(QSize(rzn1Logo->width(), rzn1Logo->height()));

	QBoxLayout *logoLayout = new QVBoxLayout;
	logoLayout->addWidget(rzn1Logo);
	logoLayout->setAlignment(rzn1Logo, Qt::AlignCenter);
	logoLayout->addStretch();

	QBoxLayout *topLayout = new QHBoxLayout;
	topLayout->addStretch();
	topLayout->addLayout(createRzn1PerfDials());
	topLayout->addStretch();
	topLayout->addLayout(logoLayout);
	topLayout->addSpacing(20);

	/* Add the layout parts to main layout */
	mainLayout = new QVBoxLayout;
	mainLayout->addStretch();
	mainLayout->addLayout(topLayout);
	mainLayout->addStretch();
	mainLayout->addLayout(createRzn1Ports());

	QWidget *widget = new QWidget;
	widget->setLayout(mainLayout);
	setCentralWidget(widget);

	/* Setup a timer to update the performance widgets regularly */
	m_perfTimer = new QTimer(this);
	connect(m_perfTimer, SIGNAL(timeout()), this, SLOT(updatePerfData()));
	m_perfTimer->start(PERF_UPDATE_MS);

	/* Setup a timer to update the port status widgets regularly */
	m_portTimer = new QTimer(this);
	connect(m_portTimer, SIGNAL(timeout()), this, SLOT(updatePortStatus()));
	m_portTimer->start(PORT_UPDATE_MS);
}
