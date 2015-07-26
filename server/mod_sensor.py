import rrdtool


databaseFile = "temp1.rrd"
MIN_TEMP = -50
ERROR_TEMP = -999.99


def read_all(val):
  rrdtool.update(databaseFile, 'N:%s' % val)
