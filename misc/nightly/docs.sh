#!/bin/sh

function makedocs2 {

mkdir -p $1/Documentation/eng
mkdir -p $1/Documentation/rus

svn export http://localhost/svn/trunk/docs/ENG $1/Documentation/eng
svn export http://localhost/svn/trunk/docs/RUS $1/Documentation/rus

svn export http://localhost/svn/trunk/addons $1/Addons

cp docs/ClearPluginsCache.cmd docs/RestoreSettings.cmd docs/SaveSettings.cmd $1/

}

rm -fR docs

svn co http://localhost/svn/trunk/docs docs

makedocs2 outfinalnew32
makedocs2 outfinalnew64
