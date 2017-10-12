CONFIG += ordered

TEMPLATE = subdirs

SUBDIRS +=  lib \
            example \


example.depends = lib
