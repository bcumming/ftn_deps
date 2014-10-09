#!/bin/bash

grep : .depend | grep -v incf$ | grep -v "\.h$" | sed "s|:||g" | grep -v "mod " > depend
