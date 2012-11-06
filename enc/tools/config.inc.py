
from os.path import join, dirname, abspath

#: path to Encyclopedia sources root
ROOT_DIR        = abspath(join(dirname(__file__), ".."))

DEST            = join(ROOT_DIR, "build")

DEST_CHM        = join(DEST, "chm")
BUILD_CHM_LOG   = join(DEST, "tool.make_chm.log")

DEST_INET       = join(DEST, "inet")

