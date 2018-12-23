#!/bin/bash

rsync -avP --exclude '*~' -e ssh docs ggarra13@web.sourceforge.net:/home/project-web/mrviewer/htdocs

