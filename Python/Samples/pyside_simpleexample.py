# Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#

from PySide import QtGui
 
import unreal
import sys
 
APP = None
if not QtGui.QApplication.instance():
   APP = QtGui.QApplication(sys.argv)
   unreal.log("Created QApplication instance: {0}".format(APP))
 
class PySideTest(QtGui.QWidget):
   def __init__(self):
       super(PySideTest, self).__init__()
       pass
   # def closeEvent(self, event):
       # if APP:
           # sys.exit(APP.exec_())
 
tool = PySideTest()
tool.show()
