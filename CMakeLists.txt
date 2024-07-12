cmake_minimum_required(VERSION 3.5)
project(securitywrap C)
add_executable(securitywrap securitywrap.c)
install(TARGETS securitywrap DESTINATION bin)
