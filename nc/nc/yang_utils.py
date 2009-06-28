
#
# yangutils
# 
# simple file manipulation functions to assist running yangdump
# and yangdiff from an online form
# 
# manage the files in the user's work directory
#


import os


###########################################################################
#
# copy a YANG file to the workdir for the user or session
#
def copyYangFile(filename, file):
    """Copy a file from the runyang form to the hack workdir"""

    if filename.find('/') != -1:
        fn = filename.rpartition('/')
        filename = fn[2]
    else:
        if filename.find('\\') != -1:
            fn = filename.rpartition('\\')
            filename = fn[2]

    targetPath = os.getcwd() + "/ncorg/workdir/" + filename
    try:
        targetFile = open(targetPath, 'w')
    except IOError:
        return "Could not open file %s" % (targetPath)
    except:
        return "Unexpected error: %s" % (sys.exc_info()[0])
    else:
        targetFile.write(file.read())
        targetFile.close()
        return ""


#############################################################################
#
#  !!!! TEMP FIXES UNTIL USERS ADDED AND MULTIPLE WORKDIRs SUPPORTED 
#
#
# delete all the old YANG files from the workdir for the user or session
#
def deleteOldYangFiles():
    """Copy a file from the runyang form to the hack workdir"""
    targetPath = os.getcwd() + "/ncorg/workdir/*"
    result = os.system('rm -f ' + targetPath)
    return result


#############################################################################
#
# generate the correct YANG input file name to send to yangdump
#
def getYangInputFilename(filename):
    """Get a filename for the yangdump input file"""

    if filename.find('/') != -1:
        fn = filename.rpartition('/')
        filename = fn[2]
    else:
        if filename.find('\\') != -1:
            fn = filename.rpartition('\\')
            filename = fn[2]
    
    targetPath = os.getcwd() + "/ncorg/workdir/" + filename
    return targetPath


##############################################################################
#
# generate the correct results output file name to send to yangdump
#
def getYangLogFilename():
    """Get a filename for the yangdump validation results"""
    filename = "results"
    targetPath = os.getcwd() + "/ncorg/workdir/" + filename
    return targetPath


###############################################################################
#
# generate the correct results output file name to send to yangdump
#
def getYangOutputFilename():
    """Get a filename for the yangdump generated reports"""
    filename = "reports"
    targetPath = os.getcwd() + "/ncorg/workdir/" + filename
    return targetPath


##############################################################################
#
# generate the correct results output file name to send to yangdump
#
def getYangResultFilename(filename):
    """Get a full URL path for the yangdump results"""

    if filename.find('/') != -1:
        fn = filename.rpartition('/')
        filename = fn[2]
    else:
        if filename.find('\\') != -1:
            fn = filename.rpartition('\\')
            filename = fn[2]

    resultPath = "/yangdumpresults/" + filename + "/results"
    return resultPath


###############################################################################
#
# generate the correctwork directory path for the user or session
#
def getYangModpath():
    """Get the YANG work directory + the standard path"""
    workdir = os.getcwd() + "/ncorg/workdir:"
    devdir = "../netconf/modules"
    return workdir+devdir

