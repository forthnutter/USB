#!/bin/sh

cp -i Makefile-2.6 Makefile

ln -s ../parapin.c kparapin/kparapin.c
ln -s ../parapin.h kparapin/parapin.h
ln -s ../parapin-linux.h kparapin/parapin-linux.h

ln -s ../parapindriver.c parapindriver/parapindriver.c
ln -s ../parapindriver.h parapindriver/parapindriver.h
ln -s ../parapin.h parapindriver/parapin.h
