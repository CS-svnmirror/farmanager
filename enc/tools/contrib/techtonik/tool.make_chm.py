# -*- coding: windows-1251 -*-
"""
Make projects files for building Far Manager Encyclopedia in .CHM format
"""

# pythonized by techtonik // gmail.com


execfile("config.inc.py")

from os import makedirs, walk
from os.path import isdir, join
from string import Template
import shutil
import logging
import subprocess
import re
import operator

logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)-6s %(message)s")
logging.addLevelName("WARN", 30)

#: just a shortcut
log = logging.info
warn = logging.warn



def make_chm_lang(lang):
  """@param lang : either 'rus' or 'eng'"""
  if lang not in ['rus', 'eng']: raise Exception("Invalid parameter")
  lang_code = lang[0:2]

  log("------------------------------------")
  log("preparing %s " % lang_code)

  command = "svn export %s/enc_%s %s/%s" % (ROOT_DIR, lang, DEST_CHM, lang_code)
  subprocess.call(command)

  chm_lang_dir = join(DEST_CHM, lang_code)
  makedirs(join(chm_lang_dir, "html"))
  makedirs(join(DEST_CHM, lang_code, "distr_chm_plugins"+lang[0]))
  # makedirs(join(DEST_CHM, lang_code, "meta"))


  # build empty directory tree
  chm_meta_dir = join(chm_lang_dir, "meta")
  chm_html_dir = join(chm_lang_dir, "html")
  for root, dirs, files in walk(chm_meta_dir):
    for d in dirs: 
      #print join(root.replace(chm_meta_dir, chm_html_dir), d)
      makedirs(join(root.replace(chm_meta_dir, chm_html_dir), d))


  log("-- translating meta into html")
  # filter files and replace "win32/.." links with calls to MSDN
  link_match = re.compile(r'href[\s"\'=\/\.]*?win32\/(?P<funcname>[^"\']*?)(\.html)?[\'"].*?>(?P<linkend>.*?<\/a>)', re.I)
  link_replace = Template(
'''
href=JavaScript:link$id.Click()>\g<linkend>
<object id=link$id type="application/x-oleobject" classid="clsid:adb880a6-d8ff-11cf-9377-00aa003b7a11">
<param name="Command" value="KLink">
<param name="DefaultTopic" value="">
<param name="Item1" value="">
<param name="Item2" value="\g<funcname>">
</object>
''')
  id = 1
  for root, dirs, files in walk(chm_meta_dir):
    for f in files:
      # todo : add a link - "report bug on this page" with a simple 2+2 captcha
      infile  = open(join(root, f))
      outfile = open(join(root.replace(chm_meta_dir, chm_html_dir), f), "w")
      for line in infile:
        while link_match.search(line):
          line = link_match.sub(link_replace.substitute(id=id), line)
          id += 1
        outfile.write(line)
      infile.close()
      outfile.close()
  log("total %d win32 links" % id)

  log("-- cleaning meta")
  shutil.rmtree(chm_meta_dir)

  
  log("-- creating CHM indexes")
  # indexes are extracted from <h1> and <a name="">..</a>
  # articles are not included in index

  index_filename = join(chm_lang_dir, "plugins%s.hhk" % lang[0])
  match_h1 = re.compile("<h1>(?P<title>.*?)</h1>", re.I)
  match_aname = re.compile(r'<a.+?name\s*=\s*(?P<quote>[\'\"])(.+?)(?P=quote).*?>(.+?)</a>', re.I)
  strip_re = re.compile(r'&quot;|[/<>\'"]', re.I)
  title_list = []
  macro_list = []
  for root, dirs, files in walk(chm_html_dir):
    if root.startswith(join(chm_html_dir, "articles")):
      continue
    macro_flag = "macro" in root
    for fn in files:
      if not fn.endswith(".html") or fn in ["faq.html", "notfound.html"]:
        continue
      relflink = join(root[root.find("html"):], fn).replace('\\', '/')
      f = open(join(root, fn))
      print chr(8)+".",
      for line in f:
        if not macro_flag:
          target_list = title_list
        else:
          target_list = macro_list
        for rh in match_h1.findall(line):
          target_list.append([relflink, strip_re.sub("", rh)])
        for ra in match_aname.findall(line):
          target_list.append([relflink+"#"+ra[1], strip_re.sub("", ra[2])])
      f.close()
  print
  
  titles = [t[1] for t in title_list]
  for ix, iv in enumerate(macro_list):
    if iv[1] in titles:
      macro_list[ix][1] = iv[1]+" (Macros)"

  title_list.sort(key=operator.itemgetter(1))
  macro_list.sort(key=operator.itemgetter(1))
  title_list.extend(macro_list)

  # processing duplicates
  for i,x in enumerate(title_list):
    if i and x[1] == title_list[i-1][1]:
      # non-anchored link from <h1> take precendence if filename is the same
      if "#" in x[0]:
        dup1 = x[0]
        dup2 = title_list[i-1][0]
      else:
        dup1 = title_list[i-1][0]
        dup2 = x[0]
      if "#" not in dup1 or "#" in dup2 or not (dup1.startswith(dup2) and len(dup2) == dup1.find("#")):
        warn("duplicate index : %s" % (x[1]))
        warn("  %s" % (x[0]))
        warn("  %s" % (title_list[i-1][0]))
      else:
        # remove anchored duplicate
        if dup1 == x[0]:
          del title_list[i]
        else:
          del title_list[i-1]

  # fcreep: mark all macros with (Macros) explicitly
  # fcreep: add distinguished meta labels to macros

  idx = open(index_filename, "w")
  idx.write(
"""<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="Microsoft&reg; HTML Help Workshop 4.1">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<UL>
""")

  for title in title_list:
    idx.write(
"""  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="%s">
    <param name="Local" value="%s">
    </OBJECT>
""" % (title[1], title[0]))

  idx.write(
"""</UL>
</BODY></HTML>
""")
# end def make_chm_lang(lang):



log("preparing CHM build")
log("-- cleaning build dir")
if isdir(DEST): shutil.rmtree(DEST)
makedirs(DEST)
logfile = logging.FileHandler(BUILD_LOG, "w")
logging.getLogger().addHandler(logfile)


log("-- making directory tree")
makedirs(DEST_CHM)

make_chm_lang("rus")
make_chm_lang("eng")

log("-- done. check build log at %s" % BUILD_LOG)
