# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Machine_Vending)

target_sources(app PRIVATE app/main.c)

target_sources(app PRIVATE app/main.c ../Machine_Vending/src-gen/VM.c)

target_include_directories(app PRIVATE ../Machine_Vending/src-gen ../VendMachine/src)
