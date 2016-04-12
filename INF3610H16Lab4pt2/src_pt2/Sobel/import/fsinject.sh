#!/bin/bash

TARGET=./home/root/tb
INJECTED_FILES_PATH=$(realpath ../../../../../../../../import/img/)
SOURCE=${INJECTED_FILES_PATH}/*

mkdir -p ${TARGET}
cp -R -v ${SOURCE} ${TARGET}