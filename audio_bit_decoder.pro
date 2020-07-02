TEMPLATE = subdirs
SUBDIRS = common encoder decoder
encoder.depends = common
decoder.depends = common
