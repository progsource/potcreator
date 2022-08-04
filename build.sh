#!/bin/bash
git submodule update --init --recursive
scons -j6
