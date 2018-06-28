AM_CXXFLAGS = --pedantic -Wall -std=c++14 -O3 -DSYSCONFDIR='"$(sysconfdir)/GPG_IP.conf"' -DCARNAME='"C0"' -DCAR0ADDR='"B8:27:EB:6A:C6:A8"' -DBTCONNECT='connect_BT_server'

AM_LDFLAGS =

noinst_LIBRARIES = libGoPiGo3.a libtof.a
libGoPiGo3_a_SOURCES = libs/GoPiGo3.cpp

libtof_a_SOURCES = libs/tof.c
libtof_a_CXXFLAGS = --pedantic -Wall -std=c11 -O3

bin_PROGRAMS = GoPiGo3Demo
GoPiGo3Demo_SOURCES = main.cpp distance.cpp bluetooth.cpp udp.cpp udp.h bluetooth.h distance.h
GoPiGo3Demo_LDADD = libGoPiGo3.a libtof.a
GoPiGo3Demo_LDFLAGS = -lbluetooth -lpthread
