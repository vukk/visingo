TEMPLATE = subdirs

CONFIG += ordered

# include SUBDIRS for qt pods
include(pods-subdirs.pri)

# visingoapp must be last
SUBDIRS += visingoapp
